/*
  Twitter Writer (Socket API)
 
  This sketch connects to Twitter and posts a message on
  http://twitter.com/RedFlyShield

  Based on the Twitter Lib from NeoCat
  http://www.arduino.cc/playground/Code/TwitterLibrary
  http://arduino-tweet.appspot.com

  Requires RedFly (CS Pin4) + mSD-shield with MI0283QT-Adapter and the 
  PS2Keyboard Library http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
 */

#include <Wire.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>
#include <PS2Keyboard.h>
#include <RedFly.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)

PS2Keyboard keyboard;
/*
byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
*/
byte server[]    = {   0,  0,  0,  0 }; //ip from server

#define HOSTNAME "arduino-tweet.appspot.com" //host
#define TOKEN    "273978908-s6eBqQrr97iXcrXVw4abHcpZ0bof2v5mKdQANXEI" //token from twitter.com/RedFlyShield

char message[256]; //min. 141
uint8_t m_pos=0;


//display output functions
#define infoClear()  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(255,255,255))
#define infoText(x)  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,255,255), RGB(0,0,0), 1)
#define errorText(x) lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,0,0), RGB(0,0,0), 1)


uint8_t clearall(void)
{
  lcd.fillScreen(RGB(255,255,255));
  lcd.fillRect(0, 0, lcd.getWidth(), 20, RGB(0,0,0));
  lcd.drawTextPGM(20, 3, PSTR("Write a message and press Enter..."), RGB(255,255,255), RGB(0,0,0), 1);
}


void setup()
{
  //init display
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.fillScreen(RGB(255,255,255));

  //init RedFly
  RedFly.init(115200, HIGH_POWER);

  //init keyboard
  keyboard.begin(5, 3, PS2Keymap_German); //Data, Clock/IRQ (Uno: 2 3 | Mega: 2 3 18 19 20 21)

  //draw screen background
  clearall();

  //set print options
  lcd.setCursor(2, 25);
  lcd.setTextColor(RGB(0,0,0), RGB(255,255,255));
  lcd.setTextSize(2);
}


uint8_t start_wifi(void)
{
  uint8_t ret, i;

  //init the WiFi module
  infoText("WiFi...");
  ret = RedFly.init(115200, HIGH_POWER); //baud rate, LOW_POWER MED_POWER HIGH_POWER
  if(ret)
  {
    errorText("WiFi...Error"); //there are problems with the communication between the Arduino and the RedFly
    return 1;
  }

  //join network
  for(i=5; i!=0; i--) //try 5 times
  {
    //scan for wireless networks (must be run before join command)
    infoText("Scan...");
    RedFly.scan();

    //join network
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER or IBSS_CREATOR, chn, authmode) //join infrastructure or ad-hoc network, or create ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw", IBSS_CREATOR, chn) //create ad-hoc network with password, channel 1-14
    // ret = join("wlan-ssid", IBSS_CREATOR, chn) //create ad-hoc network, channel 1-14
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network with password
    // ret = join("wlan-ssid", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw") //join infrastructure network with password
    // ret = join("wlan-ssid") //join infrastructure network
    infoText("Join...");
    ret = RedFly.join("wlan-ssid", "wlan-passw", INFRASTRUCTURE);
    if(ret)
    {
      errorText("Join...Error");
    }
    else
    {
      break;
    }
  }
  if(ret)
  {
    return 2;
  }

  //set ip config
  // ret = RedFly.begin(); //DHCP
  // ret = RedFly.begin(1 or 2); //1=DHCP or 2=Auto-IP
  // ret = RedFly.begin(ip);
  // ret = RedFly.begin(ip, dnsserver);
  // ret = RedFly.begin(ip, dnsserver, gateway);
  // ret = RedFly.begin(ip, dnsserver, gateway, netmask);
  infoText("IP...");
  ret = RedFly.begin();
  if(ret)
  {
    errorText("IP...Error");
    RedFly.disconnect();
    return 3;
  }

  infoClear();

  return 0;
}


uint8_t send_msg(void)
{
  uint8_t http, sock, buf[32];
  uint16_t rd, len, msg_len;

  if(RedFly.getlocalip(buf) != 0) //RedFly connected?
  {
    return 1;
  }

  infoText("Get IP...");
  if(RedFly.getip(HOSTNAME, server) == 0) //get ip
  {
    infoText("Connecting...");
    http = RedFly.socketConnect(PROTO_TCP, server, 80); //start connection to server on port 80
    if(http != INVALID_SOCKET)
    {
      infoText("Send message...");
      //build message and send to Twitter app
      RedFly.socketSendPGM(http, PSTR("POST http://"HOSTNAME"/update HTTP/1.0\r\nHost: "HOSTNAME"\r\nContent-Length: ")); //send HTTP header
      sprintf((char*)buf, "%i\r\n\r\n", strlen("token="TOKEN"&status=")+strlen(message)); //calc content length
      RedFly.socketSend(http, (char*)buf); //send content length and HTTP header end
      RedFly.socketSendPGM(http, PSTR("token="TOKEN"&status=")); //send token
      RedFly.socketSend(http, message); //send message
      delay(100);

      //get response
      for(msg_len=0, message[0]=0;;)
      {
        sock = 0xFF; //0xFF = return data from all open sockets
        rd = RedFly.socketRead(&sock, &len, buf, sizeof(buf));
        if(sock == http)
        {
          if((rd != 0) && (rd != 0xFFFF))
          {
            if((msg_len+rd) > sizeof(message))
            {
              rd = sizeof(message)-msg_len;
            }
            memcpy(&message[msg_len], buf, rd);
            msg_len += rd;
          }
          if((rd == 0xFFFF) || (len == 0)) //connection closed or all data received
          {
            break;
          }
        }
        else if(RedFly.socketClosed(http))
        {
          break;
        }
      }
      message[msg_len] = 0;

      //print response
      infoClear();
      lcd.setCursor(2, 25);
      lcd.setTextSize(1);
      lcd.print(message);
      lcd.setTextSize(2);
      delay(5000);

      RedFly.socketClose(http);
      RedFly.socketReset();
    }
    else
    {
      errorText("Connecting...Error");
      return 3;
    }
  }
  else
  {
    errorText("Get IP...Error");
    return 2;
  }

  infoClear();

  return 0;
}


void loop()
{
  char c;
  uint8_t i;

  if(keyboard.available())
  {
    c = keyboard.read();
    switch(c)
    {
      case PS2_ENTER:
        if(m_pos != 0)
        {
          while(send_msg())
          {
            while(start_wifi() != 0){ delay(1000); } //restart wifi connection
          }
          message[0] = 0;
          m_pos      = 0;
          clearall();
          lcd.setCursor(2, 25);
          while(keyboard.read() != -1); //clear keyboard buffer
        }
        break;

      case PS2_ESC:
        if(m_pos != 0)
        {
          memset(message, 0, sizeof(message));
          m_pos = 0;
          clearall();
          lcd.setCursor(2, 25);
          while(keyboard.read() != -1); //clear keyboard buffer
        }
        break;

      case PS2_DELETE:
        if(m_pos > 0)
        {
          message[--m_pos] = 0;
          lcd.fillRect(0, 20, lcd.getWidth(), lcd.getHeight()-20, RGB(255,255,255));
          lcd.setCursor(2, 25);
          lcd.print(message);
          while(keyboard.read() != -1); //clear keyboard buffer
        }
        break;

      default:
        if((c >= '0') && (c <= '9') ||
           (c >= 'A') && (c <= 'Z') ||
           (c >= 'a') && (c <= 'z') ||
           (c == ' ')               ||
           (c == '@')               ||
           (c == '#')               ||
           (c == '-')               ||
           (c == '+')               ||
           (c == '*')               ||
           (c == ',')               ||
           (c == '.')               ||
           (c == '!')               ||
           (c == '?'))
        {
          if(m_pos < 140)
          {
            message[m_pos++] = c;
            message[m_pos]   = 0;
            lcd.print(c);
          }
        }
        break;
    }
  }
}
