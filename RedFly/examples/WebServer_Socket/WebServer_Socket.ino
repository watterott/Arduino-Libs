/*
  Web Server (Socket API)
  
  This sketch acts as a server using a RedFly-Shield.
  For testing, open in your browser http://192.168.0.30/
  (Replace the IP with the one from your RedFly-Shield.)
 */

#include <RedFly.h>


byte ip[]      = { 192,168,  0, 30 }; //ip from shield (server)
byte netmask[] = { 255,255,255,  0 }; //netmask

uint8_t http=INVALID_SOCKET; //socket handle
uint16_t http_len=0; //receive len
char http_buf[64];   //receive buffer


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
        //RedFly.getlocalip(ip); //receive shield IP in case of DHCP/Auto-IP
        http = RedFly.socketListen(PROTO_TCP, 80); //start listening on port 80
        if(http == 0xFF)
        {
          debugoutln("SOCKET ERR");
          RedFly.disconnect();
          for(;;); //do nothing forevermore
        }
      }
    }
  }
}


void loop()
{
  uint8_t sock;
  char buf[512], *ptr;
  uint16_t buf_len, rd, len;

  //check if socket is opened
  if(RedFly.socketClosed(http)) //no socket open
  {
    http = RedFly.socketListen(PROTO_TCP, 80); //start listening on port 80
  }

  //get data
  sock    = 0xFF; //0xFF = return data from all open sockets
  ptr     = buf;
  buf_len = 0;
  do
  {
    rd = RedFly.socketRead(&sock, &len, (uint8_t*)ptr, sizeof(buf)-buf_len);
    if((rd != 0) && (rd != 0xFFFF)) //0xFFFF = connection closed
    {
      ptr     += rd;
      buf_len += rd;
    }
  }while((len != 0) && (buf_len < sizeof(buf)));

  //progress the received data
  if(buf_len && (sock == http) && (sock != INVALID_SOCKET))
  {
    if(strncmp_P(buf, PSTR("GET / HTTP"), 10) == 0) //main site
    {
      //send HTTP 200 header
      RedFly.socketSendPGM(sock, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")); //Content-Length: xx\r\n
      //send HTML data
      RedFly.socketSendPGM(sock, PSTR("Hello, World! <br><br>"));

      //output the value of each analog input pin
      for(int chn=0; chn < 6; chn++)
      {
        sprintf((char*)buf, "analog input %i is %i <br>", chn, analogRead(chn));
        RedFly.socketSend(sock, (char*)buf);
      }

      //show LED on/off link
      RedFly.socketSendPGM(sock, PSTR("<br><br> <a href='/ledon'>LED on</a> - <a href='/ledoff'>LED off</a>"));
    }
    else if(strncmp_P(buf, PSTR("GET /ledon HTTP"), 15) == 0) //led on
    {
      //switch LED on
      pinMode(13, OUTPUT);
      digitalWrite(13, HIGH);

      //send HTTP 200 header
      RedFly.socketSendPGM(sock, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
      //send HTML data
      RedFly.socketSendPGM(sock, PSTR("Hello, World! <br><br>"));
      RedFly.socketSendPGM(sock, PSTR("<a href='/ledoff'>LED off</a> <br><br>"));
    }
    else if(strncmp_P(buf, PSTR("GET /ledoff HTTP"), 16) == 0) //led off
    {
      //switch LED off
      pinMode(13, OUTPUT);
      digitalWrite(13, LOW);

      //send HTTP 200 header
      RedFly.socketSendPGM(sock, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
      //send HTML data
      RedFly.socketSendPGM(sock, PSTR("Hello, World! <br><br>"));
      RedFly.socketSendPGM(sock, PSTR("<a href='/ledon'>LED on</a> <br><br>"));
    }
    else
    {
      //send HTTP 404 header
      RedFly.socketSendPGM(sock, PSTR("HTTP/1.1 404 Not Found\r\n\r\n"));
    }

    //close connection
    RedFly.socketClose(sock);
  }
}
