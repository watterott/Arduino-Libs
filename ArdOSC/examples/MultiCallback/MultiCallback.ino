#include <RedFly.h>
#include <ArdOSC.h>


byte myIp[] = { 192, 168, 0, 30 };
uint16_t serverPort = 10000;
uint16_t destPort = 12000;
  
OSCServer server;
OSCClient client;

int v1=0;
int v2=10;
int v3=20;


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
       server.addCallback("/ard/aaa",&func1);
       server.addCallback("/ard/bbb",&func2);
       server.addCallback("/ard/abcde",&func3);
      }
    }
  }
}


void loop()
{
  if(server.aviableCheck()>0)
  {
    debugoutln("alive!"); //callback after process
  }
}


void func1(OSCMessage *_mes)
{
//  logMes(_mes);
  
  //create new osc message
  OSCMessage newMes;
  
  //set destination ip address & port no
  newMes.setAddress(_mes->getIpAddress(),destPort);
  
  if(v1++>1000) v1=0;
  //set argument
  newMes.beginMessage("/ard/aaa");
  newMes.addArgInt32(v1);
  newMes.addArgString("function1!");

  //send osc message
  server.stop(); //stop server while sending
  client.send(&newMes);
  server.begin(serverPort); //start server


void func2(OSCMessage *_mes)
{
//  logMes(_mes);
  
  //create new osc message
  OSCMessage newMes;
  
  //set destination ip address & port no
  newMes.setAddress(_mes->getIpAddress(),destPort);
  
  if(v2++>1000) v2=0;
  
  //set argument
  newMes.beginMessage("/ard/bbb");
  newMes.addArgInt32(v2);
  newMes.addArgString("function2!");

  //send osc message
  server.stop(); //stop server while sending
  client.send(&newMes);
  server.begin(serverPort); //start server
}


void func3(OSCMessage *_mes)
{
// logMes(_mes);

  //create new osc message
  OSCMessage newMes;
  
  //set destination ip address & port no
  newMes.setAddress(_mes->getIpAddress(),destPort);
  
   if(v3++>1000) v3=0;
   
  //set argument
  newMes.beginMessage("/ard/abcde");
  newMes.addArgInt32(v3);
  newMes.addArgString("function3!");

  //send osc message
  server.stop(); //stop server while sending
  client.send(&newMes);
  server.begin(serverPort); //start server
}


void logMes(OSCMessage *_mes)
{
  logIp(_mes);
  logOscAddress(_mes);
  
  //get source ip address
  byte *sourceIp = _mes->getIpAddress();

  //get 1st argument(string)
  int strSize=_mes->getArgStringSize(0);
  char *str;
  str = (char *)calloc(strSize, 1);
  _mes->getArgString(0, str); 
  
  debugoutln(str);

  //delete string buffer
  free(str);
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
