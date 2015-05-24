/*
  Xively Client (previously Cosm, Pachube)
 
  This sketch connects to Xively (http://xively.com) using a RedFly-Shield.
  https://xively.com/feeds/81548
 */

#include <digitalWriteFast.h>
#include <RedFly.h>
#include <RedFlyClient.h>


byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
byte server[]    = {   0,  0,  0,  0 }; //ip from api.xively.com (server)

#define HOSTNAME  "api.xively.com"         //host
#define APIKEY    "ZmR191JkoUttt0rQoCSqVKdKa5KSAKx5c1JDZGZ1OFFyYz0g" //your api key
#define FEEDID    "81548"                  //your feed ID
#define USERAGENT "My Arduino Project"     //user agent is the project name
#define INTERVAL 10*1000UL                 //10s delay between updates

unsigned long lastConnectionTime = 0; //last time you connected to the server, in milliseconds

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
          //everything okay
          debugoutln("Start sending...");
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
  unsigned long ms = millis();

  if(!client.connected() && ((ms-lastConnectionTime) > INTERVAL))
  {
    lastConnectionTime = ms;

    //read and pepare sensor data
    char tmp[64];
    int adc0 = analogRead(A0);
    int adc1 = analogRead(A1);  
    int adc2 = analogRead(A2);  
    sprintf(tmp, "adc0,%i\r\nadc1,%i\r\nadc2,%i\r\n", adc0, adc1, adc2);

    //connect to server
    if(client.connect(server, 80)) 
    {
      //send HTTP header
      client.print_P(PSTR("PUT /v2/feeds/"FEEDID".csv HTTP/1.1\r\n"));
      client.print_P(PSTR("Host: "HOSTNAME"\r\n"));
      client.print_P(PSTR("X-ApiKey: "APIKEY"\r\n"));
      client.print_P(PSTR("User-Agent: "USERAGENT"\r\n"));
      client.print_P(PSTR("Content-Type: text/csv\r\n"));
      client.print_P(PSTR("Content-Length: "));
      client.println(strlen(tmp));
      client.print_P(PSTR("Connection: close\r\n\r\n"));
      //here's the content of the PUT request
      client.print(tmp);
      //close connection
      client.stop();
    }
    else
    {
      debugoutln("CONN ERR");
    }
  }
}
