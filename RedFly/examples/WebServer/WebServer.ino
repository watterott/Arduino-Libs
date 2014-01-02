/*
  Web Server
  
  This sketch acts as a server using a RedFly-Shield.
  For testing, open in your browser http://192.168.0.30/
  (Replace the IP with the one from your RedFly-Shield.)
 */

#include <RedFly.h>
#include <RedFlyServer.h>


byte ip[]      = { 192,168,  0, 30 }; //ip from shield (server)
byte netmask[] = { 255,255,255,  0 }; //netmask

//initialize the server library with the port 
//you want to use (port 80 is default for HTTP)
RedFlyServer server(80);


//debug output functions (9600 Baud, 8N2)
//Leonardo boards use USB for communication, so we dont need to disable the RedFly
void debugout(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.print(s);
#else
  RedFly.disable();
  Serial.print(s);
  RedFly.enable();
#endif
}

void debugoutln(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.println(s);
#else
  RedFly.disable();
  Serial.println(s);
  RedFly.enable();
#endif
}


void setup()
{
  uint8_t ret;

#if defined(__AVR_ATmega32U4__) //Leonardo boards use USB for communication
  Serial.begin(9600); //init serial port and set baudrate
  while(!Serial); //wait for serial port to connect (needed for Leonardo only)
#endif

  //init the WiFi module on the shield
  // ret = RedFly.init(br, pwr) //br=9600|19200|38400|57600|115200|200000|230400, pwr=LOW_POWER|MED_POWER|HIGH_POWER
  // ret = RedFly.init(pwr) //9600 baud, pwr=LOW_POWER|MED_POWER|HIGH_POWER
  // ret = RedFly.init() //9600 baud, HIGH_POWER
  ret = RedFly.init();
  if(ret)
  {
    debugoutln("INIT ERR"); //there are problems with the communication between the Arduino and the RedFly
  }
  else
  {
    //scan for wireless networks (must be run before join command)
    RedFly.scan();

    //join network
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER or IBSS_CREATOR, chn, authmode) //join infrastructure or ad-hoc network, or create ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw", IBSS_CREATOR, chn) //create ad-hoc network with password, channel 1-14
    // ret = join("wlan-ssid", IBSS_CREATOR, chn) //create ad-hoc network, channel 1-14
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network with password
    // ret = join("wlan-ssid", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw") //join infrastructure network with password
    // ret = join("wlan-ssid") //join infrastructure network
    ret = RedFly.join("wlan-ssid", "wlan-passw", INFRASTRUCTURE);
    if(ret)
    {
      debugoutln("JOIN ERR");
      for(;;); //do nothing forevermore
    }
    else
    {
      //set ip config
      // ret = RedFly.begin(); //DHCP
      // ret = RedFly.begin(1 or 2); //1=DHCP or 2=Auto-IP
      // ret = RedFly.begin(ip);
      // ret = RedFly.begin(ip, dnsserver);
      // ret = RedFly.begin(ip, dnsserver, gateway);
      // ret = RedFly.begin(ip, dnsserver, gateway, netmask);
      ret = RedFly.begin(ip, 0, 0, netmask);
      if(ret)
      {
        debugoutln("BEGIN ERR");
        RedFly.disconnect();
        for(;;); //do nothing forevermore
      }
      else
      {
        RedFly.getlocalip(ip); //receive shield IP in case of DHCP/Auto-IP
        server.begin();
      }
    }
  }
}


void loop()
{
  //listen for incoming clients
  if(server.available())
  {
    //a http request ends with a blank line
    boolean currentLineIsBlank = true;
    while(server.available())
    {
      char c = server.read();
      //if you've gotten to the end of the line (received a newline
      //character) and the line is blank, the http request has ended,
      //so you can send a reply
      if(c == '\n' && currentLineIsBlank)
      {
        //clear input buffer
        server.flush(); 

        //send standard HTTP 200 header
        server.print_P(PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));

        //send some text
        server.println_P(PSTR("Hello, World! <br><br>"));

        //output the value of each analog input pin
        for(int chn=0; chn < 6; chn++)
        {
          char tmp[32];
          sprintf_P(tmp, PSTR("analog input %i is %i <br>"), chn, analogRead(chn));
          server.print(tmp);
        }

        //show IP address of RedFly
        server.println_P(PSTR("<br><i>RedFly IP: "));
        server.print(ip[0], DEC); server.print(".");
        server.print(ip[1], DEC); server.print(".");
        server.print(ip[2], DEC); server.print(".");
        server.print(ip[3], DEC);
        server.println_P(PSTR("<br></i>"));
        break;
      }
      if(c == '\n')
      {
        //you're starting a new line
        currentLineIsBlank = true;
      } 
      else if(c != '\r')
      {
        //you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }

    //close connection
    server.stop();
  }
  else if(!server.connected()) //listening port still open?
  {
    server.stop(); //stop and reset server
    server.begin(); //start server
  }
}
