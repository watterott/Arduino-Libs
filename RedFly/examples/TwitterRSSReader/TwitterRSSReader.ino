/*
  Twitter RSS Reader

  This sketch reads RSS feeds from Twitter and shows them on a display.
  Requires RedFly + mSD-shield with MI0283QT-Adapter.
 */

#include <Wire.h>
#include <SPI.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>
#include <RedFly.h>
#include <RedFlyClient.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)


/*
byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
*/
byte server[]    = {   0,  0,  0,  0 }; //ip from server

RedFlyClient client(server, 80);

//obsolete Twitter API
// #define HOSTNAME "twitter.com"
// #define FILENAME "statuses/user_timeline/redflyshield.rss?count=5"

//alternative 1: http://www.rssitfor.me/getrss?name=USERNAME
// #define HOSTNAME "rssitfor.me"
// #define FILENAME "getrss?name=redflyshield?count=5"

//alternatives 2: http://twfeed.com/feed/USERNAME or http://twfeed.com/atom/USERNAME
 #define HOSTNAME "twfeed.com"
 #define FILENAME "feed/redflyshield?count=5"


//display output functions
#define infoClear()  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(255,255,255))
#define infoText(x)  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,255,255), RGB(0,0,0), 1)
#define errorText(x) lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,0,0), RGB(0,0,0), 1)


uint8_t start_wifi(void)
{
  uint8_t ret, i;

  //init the WiFi module
  infoText("WiFi...");
  ret = RedFly.init(115200, HIGH_POWER); //baudrate, LOW_POWER MED_POWER HIGH_POWER
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

  return 0;
}


void setup()
{
  uint8_t ret;

  //init display
  //init display
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.touchStartCal(); //calibrate touchpanel

  //start WiFi
  while(start_wifi() != 0){ delay(1000); }

  //draw screen background
  lcd.fillRect(0, 0, lcd.getWidth(), 20, RGB(0,0,0));
  lcd.drawTextPGM((lcd.getWidth()/2)-20, 5, PSTR("newer"), RGB(255,255,255), RGB(0,0,0), 1);
  lcd.fillRect(0, lcd.getHeight()-20, lcd.getWidth(), 20, RGB(0,0,0));
  lcd.drawTextPGM((lcd.getWidth()/2)-20, lcd.getHeight()-15, PSTR("older"), RGB(255,255,255), RGB(0,0,0), 1);

  //send request
  while(request_data() != 0)
  {
    delay(1000);
    start_wifi(); //restart wifi connection
  }
}


char data[128];  //receive buffer
unsigned int dlen=0; //receive buffer length
uint8_t state=0, lastc1=0, lastc2=0, lastc3=0, rss_item=1, item=0;


uint8_t request_data() //send request to server
{
  uint8_t i;
  unsigned long ms, last_time;
  char txt[8];

  //draw item index
  sprintf(txt, "%i", ((rss_item+1)/2));
  lcd.drawText((lcd.getWidth()/2)-10, 50, txt, RGB(0,0,255), RGB(255,255,255), 2);

  infoText("Get IP...");
  if(RedFly.getip(HOSTNAME, server) != 0) //get ip
  {
    errorText("Get IP...Error");
  }
  else
  {
    for(i=0, last_time=0; client.connected() == 0;)
    {
      ms = millis();
      if(((ms-last_time) > 3000)) //every 3s
      {
        last_time = ms; //save time
        infoText("Connecting...");
        if(client.connect(server, 80) == 0)
        {
          errorText("Connecting...Error");
          if(++i > 5)
          {
            return 2;
          }
        }
      }
    }

    for(i=0, last_time=0; client.available() == 0;)
    {
      ms = millis();
      if(((ms-last_time) > 3000)) //every 3s
      {
        last_time = ms; //save time
        infoText("Request...");
        client.print_P(PSTR("GET /"FILENAME" HTTP/1.0\r\nHost: "HOSTNAME"\r\n\r\n")); //Connection: close\r\n
        i++;
      }
      if((i > 5) || (client.connected() == 0))
      {
        errorText("Request...Error");
        client.stop();
        return 3;
      }
    }
    infoText("Receive...");
  }

  return 0;
}


void read_data() //receive data from server
{
  int c;

  switch(state)
  {
    case 0: //search http header end (data start)
      for(;;)
      {
        c = client.read();
        if(c == -1)
        {
          break;
        }
        else if((lastc1 == '\n') && (c == '\r'))
        {
          infoText("Header found...");
          state++;
          break;
        }
        lastc3 = lastc2;
        lastc2 = lastc1;
        lastc1 = c;
      }
      break;

    case 1: //search item
      for(;;)
      {
        c = client.read();
        if(c == -1)
        {
          break;
        }
        else if((lastc3 == 'a') && (lastc2 == 'r') && (lastc1 == 'y') && (c == '>')) //new: <summary> old:<title> and <description>
        {
          if(++item == rss_item)
          {
            infoText("Tag found...");
            state++;
            break;
          }
        }
        lastc3 = lastc2;
        lastc2 = lastc1;
        lastc1 = c;
      }
      break;

    case 2: //copy
      for(;;)
      {
        c = client.read();
        if(c == -1)
        {
          state++;
          break;
        }
        else if((c != '\n') && (c != '\r'))
        {
          if(dlen >= 128)
          {
            state++;
            break;
          }
          else
          {
            data[dlen++] = c;
            if((lastc1 == '<') && (c == '/')) //tag end "</"
            {
              dlen -= 2;
              data[dlen] = 0;
              state++;
              break;
            }
          }
        }
        lastc3 = lastc2;
        lastc2 = lastc1;
        lastc1 = c;
      }
      break;

    case 3: //stop receiving
      client.stop();
      break;
  }
}


void loop()
{
  if(client.available()) //data availavle?
  {
    read_data();
  }
  else if(dlen && !client.connected()) //if the server's disconnected, stop the client
  {
    client.stop();
    data[dlen] = 0;
    lcd.fillRect(2, 25, lcd.getWidth()-2, lcd.getHeight()-50, RGB(255,255,255)); //clear text
    lcd.setCursor(2, 25);
    lcd.setTextColor(RGB(0,0,0), RGB(255,255,255));
    lcd.setTextSize(2);
    lcd.print(data);
    dlen = 0;
  }
  else
  {
    lcd.touchRead(); //service routine for touch panel

    //touch press?
    if(lcd.touchZ()) 
    {
      if(lcd.touchY() < 50) //newer rss item
      {
        if(rss_item > 1) //1 is the first item
          rss_item -= 2;
      }
      else if(lcd.touchY() > (lcd.getHeight()-50)) //older rss item
      {
        if(rss_item < 10)
          rss_item +=2;
      }
      
      //reset vars
      state  = 0;
      lastc1 = 0;
      lastc2 = 0;
      lastc3 = 0;
      item   = 0;
      dlen   = 0;

      //send request
      while(request_data() != 0)
      {
        delay(1000);
        start_wifi(); //restart wifi connection
      }
    }
  }
}
