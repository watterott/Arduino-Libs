/*
  UDP Test
 
  Test sketch for UDP client/server API.
 */

#include <digitalWriteFast.h>
#include <RedFly.h>
#include <RedFlyClient.h>
#include <RedFlyServer.h>


byte ip[]       = { 192,168,  0, 30 }; //ip from shield (client)
byte serverip[] = { 192,168,  0, 20 };

//initialize the libraries
RedFlyClient client(serverip, 1000);
RedFlyServer server(1000);


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
        client.connectUDP();
        client.println_P(PSTR("Hello, World!"));
        //client.stop();

        server.beginUDP();
      }
    }
  }
}


void loop()
{
  uint8_t buf[128];
  uint16_t i;

  i = client.available();
  if(i)
  {
    i = client.read(buf, i);
    buf[i] = 0;
    debugoutln("Client: ");
    debugoutln((char*)buf);
  }

  i = server.available();
  if(i)
  {
    i = server.read(buf, i);
    buf[i] = 0;
    debugoutln("Server: ");
    debugoutln((char*)buf);
  }
  else if(!server.connected()) //listening port still open?
  {
    server.stop(); //stop and reset server
    server.beginUDP(); //start server
  }
}
