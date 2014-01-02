/*
  Twitter RSS Reader

  This sketch reads RSS feeds from Twitter and shows them on a display.
  Requires RedFly + mSD-shield with MI0283QT-Adapter.
 */

#include <MI0283QT2.h> //include <MI0283QT2.h> or <MI0283QT9.h>
#include <ADS7846.h>
#include <RedFly.h>
#include <RedFlyClient.h>


#define TP_EEPROMADDR (0x00) //eeprom address for calibration data

MI0283QT2 lcd; //declare MI0283QT2 or MI0283QT9
ADS7846 tp;

byte ip[]        = { 192,168,  0, 30 }; //ip from shield (client)
byte netmask[]   = { 255,255,255,  0 }; //netmask
byte gateway[]   = { 192,168,  0,100 }; //ip from gateway/router
byte dnsserver[] = { 192,168,  0,100 }; //ip from dns server
byte server[]    = {   0,  0,  0,  0 }; //ip from server

RedFlyClient client(server, 80);

//obsolete Twitter API
// #define HOSTNAME "twitter.com"
// #define FILENAME "statuses/user_timeline/redflyshield.rss?count=5"

//alternative 1: http://www.rssitfor.me/getrss?name=USERNAME
 #define HOSTNAME "rssitfor.me"
 #define FILENAME "getrss?name=redflyshield?count=5"

//alternatives 2: http://twfeed.com/feed/USERNAME or http://twfeed.com/atom/USERNAME
// #define HOSTNAME "twfeed.com"
// #define FILENAME "feed/redflyshield"


//display output functions
#define infoClear()  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth()-1,(lcd.getHeight()/2)+5, COLOR_WHITE);
#define infoText(x)  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth()-1,(lcd.getHeight()/2)+5, COLOR_BLACK); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), 1, COLOR_WHITE, COLOR_BLACK)
#define errorText(x) lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth()-1,(lcd.getHeight()/2)+5, COLOR_BLACK); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), 1, COLOR_RED, COLOR_BLACK)


uint8_t start_wifi(void)
{
  uint8_t ret, i;

  //init the WiFi module
  infoText("Init WiFi...");
  ret = RedFly.init(115200, HIGH_POWER); //baudrate, LOW_POWER MED_POWER HIGH_POWER
  if(ret)
  {
    errorText("Init WiFi...Error"); //there are problems with the communication between the Arduino and the RedFly
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
  infoText("Set IP...");
  ret = RedFly.begin(ip, dnsserver, gateway, netmask);
  if(ret)
  {
    errorText("Set IP...Error");
    RedFly.disconnect();
    return 3;
  }

  infoClear();

  return 0;
}


void setup()
{
  uint8_t ret;

  //init display
  lcd.init(2); //spi-clk = Fcpu/2
  lcd.setOrientation(0);

  //init touch controller
  tp.init();
  tp.setOrientation(0);

  //clear screen
  lcd.clear(COLOR_WHITE);
  
  //touch-panel calibration
  tp.service();
  if(tp.getPressure() > 5)
  {
    tp.doCalibration(&lcd, TP_EEPROMADDR, 0); //dont check EEPROM for calibration data
  }
  else
  {
    tp.doCalibration(&lcd, TP_EEPROMADDR, 1); //check EEPROM for calibration data
  }

  //clear screen
  lcd.clear(COLOR_WHITE);

  //start WiFi
  while(start_wifi() != 0){ delay(1000); }

  //draw screen background
  lcd.fillRect(0, 0, lcd.getWidth()-1, 20, COLOR_BLACK);
  lcd.drawTextPGM((lcd.getWidth()/2)-20, 3, PSTR("newer"), 1, COLOR_WHITE, COLOR_BLACK);
  lcd.fillRect(0, lcd.getHeight()-20, lcd.getWidth()-1, lcd.getHeight()-1, COLOR_BLACK);
  lcd.drawTextPGM((lcd.getWidth()/2)-20, lcd.getHeight()-15, PSTR("older"), 1, COLOR_WHITE, COLOR_BLACK);

  //send request
  request_data();
}


char data[128];  //receive buffer
unsigned int dlen=0; //receive buffer length
uint8_t state=0, lastc1=0, lastc2=0, rss_item=1, item=0;


uint8_t request_data() //send request to server
{
  char txt[8];

  //clear text
  lcd.fillRect(2, 25, lcd.getWidth()-2, (lcd.getHeight()-25), COLOR_WHITE);

  //draw item index
  sprintf(txt, "%i", ((rss_item+1)/2));
  lcd.drawText((lcd.getWidth()/2)-10, 50, txt, 2, COLOR_BLUE, COLOR_WHITE);

  infoText("Get IP...");
  if(RedFly.getip(HOSTNAME, server) == 0) //get ip
  {
    infoText("Connecting...");
    if(client.connect(server, 80))
    {
      infoText("Get data...");
      client.print_P(PSTR("GET /"FILENAME" HTTP/1.1\r\nHost: "HOSTNAME"\r\n\r\n"));
    }
    else
    {
      errorText("Connecting...Error");
      return 2;
    }
  }
  else
  {
    errorText("Get IP...Error");
    return 1;
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
          state++;
          break;
        }
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
        else if((lastc2 == 'l') && (lastc1 == 'e') && (c == '>')) //new:<title> old:<description>
        {
          if(++item == rss_item)
          {
            state++;
            break;
          }
        }
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
            if((lastc1 == '<') && (c == '/')) //new:</title> old:</description>
            {
              dlen -= 2;
              data[dlen] = 0;
              state++;
              break;
            }
          }
        }
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
    infoClear();
    data[dlen] = 0;
    lcd.drawMLText(2, 25, lcd.getWidth()-2, (lcd.getHeight()-20), data, 2, COLOR_BLACK, COLOR_WHITE);
    dlen = 0;
  }
  else
  {
    tp.service(); //service routine for touch panel

    //touch press?
    if(tp.getPressure() > 3) 
    {
      if(tp.getY() < 50) //newer rss item
      {
        if(rss_item > 1) //1 is the first item
          rss_item -= 2;
      }
      else if(tp.getY() > (lcd.getHeight()-50)) //older rss item
      {
        if(rss_item < 10)
          rss_item +=2;
      }
      
      //reset vars
      state  = 0;
      lastc1 = 0;
      lastc2 = 0;
      item   = 0;
      dlen   = 0;

      //send request
      while(request_data())
      {
        while(start_wifi() != 0){ delay(1000); } //restart wifi connection
      }
    }
  }
}