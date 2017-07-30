#include <RedFly.h>
#include <ArdOSC.h>


byte myIp[] = { 192, 168, 0, 30 };
uint16_t serverPort = 10000;
uint16_t destPort = 12000;

char oscadr[] = "/ard/aaa";

OSCServer server;
OSCClient client;


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
    for(;;); //do nothing forevermore
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
      ret = RedFly.begin(myIp);
      if(ret)
      {
        debugoutln("BEGIN ERR");
        RedFly.disconnect();
        for(;;); //do nothing forevermore
      }
      else
      {
       //start server
       server.begin(serverPort);
       
       //set callback function
       server.addCallback(oscadr,&func1);
      }
    }
  }
}


void loop()
{
  if(server.aviableCheck()>0)
  {
    //debugoutln("alive!"); //callback after process
  }
}


void func1(OSCMessage *_mes)
{
  logIp(_mes);
  logOscAddress(_mes);
  
  //get source ip address
  byte *sourceIp = _mes->getIpAddress();

  //get 1st argument(int32)
  int tmpI=_mes->getArgInt32(0);
  
  //get 2nd argument(float)
  float tmpF=_mes->getArgFloat(1);
  
  //get 3rd argument(string)
  int strSize=_mes->getArgStringSize(2);
  char tmpStr[strSize]; //string memory allocation
  _mes->getArgString(2,tmpStr); 

  //create new osc message
  OSCMessage newMes;
  
  //set destination ip address & port no
  newMes.setAddress(sourceIp,destPort);
  
  //set argument
  newMes.beginMessage(oscadr);
  newMes.addArgInt32(tmpI+1);
  newMes.addArgFloat(tmpF+0.1);
  newMes.addArgString(tmpStr);
 
  //send osc message
  server.stop(); //stop server while sending
  client.send(&newMes);
  server.begin(serverPort); //start server
}


void logIp(OSCMessage *_mes)
{
  byte *ip = _mes->getIpAddress();
  char tmp[20];

  sprintf(tmp, "IP: %i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
  debugoutln(tmp);
}


void logOscAddress(OSCMessage *_mes)
{
  debugoutln(_mes->getOSCAddress());
}
