/*
  Web Client
 
  This sketch connects to a website using a RedFly-Shield. 
 */

#include <RedFly.h>
#include <RedFlyClient.h>


byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
byte server[]    = {   0,  0,  0,  0 }; //{  85, 13,145,242 }; //ip from www.watterott.net (server)

#define HOSTNAME "www.watterott.net"  //host

//initialize the client library with the ip and port of the server 
//that you want to connect to (port 80 is default for HTTP)
RedFlyClient client(server, 80);


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
      ret = RedFly.begin(ip, dnsserver, gateway, netmask);
      if(ret)
      {
        debugoutln("BEGIN ERR");
        RedFly.disconnect();
        for(;;); //do nothing forevermore
      }
      else
      {
        if(RedFly.getip(HOSTNAME, server) == 0) //get ip
        {
          if(client.connect(server, 80))
          {
            //make a HTTP request
            client.print_P(PSTR("GET / HTTP/1.1\r\nHost: "HOSTNAME"\r\n\r\n"));
          }
          else
          {
            debugoutln("CLIENT ERR");
            RedFly.disconnect();
            for(;;); //do nothing forevermore
          }
        }
        else
        {
          debugoutln("DNS ERR");
          RedFly.disconnect();
          for(;;); //do nothing forevermore
        }
      }
    }
  }
}


char data[1024];  //receive buffer
unsigned int len=0; //receive buffer length

void loop()
{
  int c;

  //if there are incoming bytes available 
  //from the server then read them 
  if(client.available())
  {
    do
    {
      c = client.read();
      if((c != -1) && (len < (sizeof(data)-1)))
      {
        data[len++] = c;
      }
    }while(c != -1);
  }

  //if the server's disconnected, stop the client and print the received data
  if(len && !client.connected())
  {
    client.stop();
    RedFly.disconnect();

    data[len] = 0;
    debugout(data);
    
    len = 0;
  }
}
