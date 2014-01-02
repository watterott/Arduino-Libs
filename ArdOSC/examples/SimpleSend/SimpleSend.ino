#include <RedFly.h>
#include <ArdOSC.h>


byte myIp[] = { 192, 168, 0, 30 };
uint16_t serverPort = 10000;
uint16_t destPort = 12000;
byte destIp[] = { 192, 168, 0, 2 };

OSCClient client;

//create new osc message
OSCMessage global_mes;
  
int v3=10;
float v4=10.0;
char str1[]="simple send 1!";

int v1=0;
float v2=0.0;


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
    }
  }
}

  
void loop()
{
  global_mes.setAddress(destIp,destPort);
  
  global_mes.beginMessage("/ard/send1");
  global_mes.addArgInt32(v1);
  global_mes.addArgFloat(v2);
  global_mes.addArgString(str1);
  
  client.send(&global_mes);
  
  global_mes.flush(); //object data clear
  
  delay(800);

  send2();
  
  delay(800);
  
  v1++;
  v2 += 0.1;
}


void send2()
{
  //loacal_mes,str is release by out of scope
  OSCMessage loacal_mes;
  
  loacal_mes.setAddress(destIp,destPort);
  
  loacal_mes.beginMessage("/ard/send2");
  loacal_mes.addArgInt32(v3);
  loacal_mes.addArgFloat(v4);
  
  char str[]="simple send2 !!";
  loacal_mes.addArgString(str);
  
  client.send(&loacal_mes);
  
  v3++;
  v4 += 0.1;
}
