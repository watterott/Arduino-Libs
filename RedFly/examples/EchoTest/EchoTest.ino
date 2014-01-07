/*
  Echo Test (Socket API)
 
  This sketch acts as a echo server using a RedFly-Shield.
  All received data will be resend to the client.
 */

#include <RedFly.h>


byte ip[]      = { 192,168,  0, 30 }; //ip from shield (server)
byte netmask[] = { 255,255,255,  0 }; //netmask

#define TCP_PORT  (80) //local TCP port on the shield
#define UDP_PORT  (80) //local UDP port on the shield

uint8_t hTCP=INVALID_SOCKET, hUDP=INVALID_SOCKET;  //socket handles


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
    }
  }
}


void loop()
{
  uint8_t sock, buf[64], *ptr;
  uint16_t buf_len, rd, len;
  uint16_t port; //incomming UDP port
  uint8_t ip[4]; //incomming UDP ip

  //check if sockets are opened
  if(RedFly.socketClosed(hTCP))
  {
    hTCP = RedFly.socketListen(PROTO_TCP, TCP_PORT); //open TCP socket
  }
  if(RedFly.socketClosed(hUDP))
  {
    hUDP = RedFly.socketListen(PROTO_UDP, UDP_PORT); //open UDP socket
  }

  //get data
  sock    = 0xFF; //0xFF = return data from all open sockets
  ptr     = buf;
  buf_len = 0;
  do
  {
    rd = RedFly.socketRead(&sock, &len, ip, &port, ptr, sizeof(buf)-buf_len);
    if((rd != 0) && (rd != 0xFFFF)) //0xFFFF = connection closed
    {
      ptr     += rd;
      buf_len += rd;
    }
  }while(len != 0);  //len contains the data size from rx buffer

  //send data
  if(buf_len && (sock != INVALID_SOCKET)) //data received for transmitting?
  {
    if(sock == hTCP)
    {
      RedFly.socketSend(hTCP, buf, buf_len); //send data to TCP socket
    }
    else if(sock == hUDP)
    {
      RedFly.socketSend(hUDP, buf, buf_len, ip, port); //send data to UDP socket
    }
  }
}
