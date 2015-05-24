/*
  NBNS (NetBIOS Name Service) Test
 
  This sketch shows the basic NetBIOS Name Service (NBNS) support in action.
  For testing, open in your browser http://redfly/
 */

#include <digitalWriteFast.h>
#include <RedFly.h>
#include <RedFlyServer.h>
#include <RedFlyNBNS.h>


byte ip[] = { 192,168,  0, 30 }; //ip from shield (client)

//initialize the server library with the port 
//you want to use (port 80 is default for HTTP)
RedFlyServer server(80);

//initialize the NBNS library with the device name (max. 16 characters)
RedFlyNBNS NBNS("REDFLY");


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
      ret = RedFly.begin(ip);
      if(ret)
      {
        debugoutln("BEGIN ERR");
        RedFly.disconnect();
        for(;;); //do nothing forevermore
      }
      else
      {
        //receive shield IP in case of DHCP/Auto-IP
        RedFly.getlocalip(ip);

        //start server
        server.begin();

        //set other device name
        // NBNS.setName("REDFLY");
        // NBNS.setNamePGM(PSTR("REDFLY"));
      }
    }
  }
}


void loop()
{
  //NBNS service routine
  NBNS.service();

  //listen for incoming clients (HTTP server)
  if(server.available())
  {
    //an http request ends with a blank line
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

        //show IP address of RedFly
        server.println_P(PSTR("My IP is: "));
        server.print(ip[0], DEC); server.print(".");
        server.print(ip[1], DEC); server.print(".");
        server.print(ip[2], DEC); server.print(".");
        server.print(ip[3], DEC);
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
