/*
  NTP (Network Time Protocol) Test
 
  This sketch connects to a NTP server and gets the current time.
 */

#include <digitalWriteFast.h>
#include <RedFly.h>


byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
byte server[]    = {   0,  0,  0,  0 }; //{ 188,138,107,156 }; //ip from pool.ntp.org (server)

#define HOSTNAME "pool.ntp.org" //host


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
          uint32_t time;
          time = RedFly.gettime(server); //get time
          if(time != 0UL)
          {
            char tmp[64];
            sprintf_P(tmp, PSTR("Time: %lu sec since 1970"), time);
            debugoutln(tmp);
          }
          else
          {
            debugoutln("NTP ERR");
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


void loop()
{

}
