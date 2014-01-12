/*
  Paint
 
  This sketch connects two RedFly-Shields with displays. 
  One Arduino has to be WORKING_MODE=0 and the other WORKING_MODE=1.
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
#include <RedFlyServer.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)

#define WORKING_MODE 0 //0=server, 1=client

byte ip_server[] = { 192,168,  0, 50 }; //ip from server shield
byte ip_client[] = { 192,168,  0, 51 }; //ip from client shield

uint16_t comm_port = 1000; //communication port

#if WORKING_MODE == 0 //server
RedFlyServer server(comm_port);
#else
RedFlyClient client(ip_server, comm_port);
#endif

uint16_t last_x=0, last_y=0, color=RGB(0,0,0);

//display output functions
#define infoClear()  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(255,255,255))
#define infoText(x)  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,255,255), RGB(0,0,0), 1)
#define errorText(x) lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,0,0), RGB(0,0,0), 1)


uint8_t clearall(uint8_t send) //redraw complete screen
{
  if(send)
  {
    uint16_t buf[4]={0xBEEF,0x0000,0x0000,0x0000};
#if WORKING_MODE == 0 //server
    server.write((uint8_t*)buf, 8);
#else
    client.write((uint8_t*)buf, 8);
#endif
  }

  lcd.fillRect(0, 0, 20, lcd.getHeight(), color);
  lcd.fillRect(lcd.getWidth()-20, 0, 20, lcd.getHeight(), RGB(0,0,0));
  lcd.fillRect(20, 0, lcd.getWidth()-40, lcd.getHeight(), RGB(255,255,255));
}


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

  //join/create network
  for(i=5; i!=0; i--) //try 5 times
  {
#if WORKING_MODE == 0 //server
    infoText("Create network...");
    ret = RedFly.join("PAINT", "1A2B3C4D5E", IBSS_CREATOR, 1); //create ad-hoc network
#else
    infoText("Scan...");
    RedFly.scan(); //scan for wireless networks (must be run before join command)
    infoText("Join...");
    ret = RedFly.join("PAINT", "1A2B3C4D5E", IBSS_JOINER); //join ad-hoc network
#endif
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
  infoText("IP...");
#if WORKING_MODE == 0 //server
  ret = RedFly.begin(ip_server);
#else
  ret = RedFly.begin(ip_client);
#endif
  if(ret)
  {
    errorText("IP...Error");
    RedFly.disconnect();
    return 3;
  }

#if WORKING_MODE == 0 //server
  infoText("Server...");
  server.begin();
#else
  infoText("Client...");
  client.connect();
#endif

  clearall(1);

  return 0;
}


void setup()
{
  //init display
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.touchStartCal(); //calibrate touchpanel

  //show working mode
#if WORKING_MODE == 0 //server
  lcd.drawTextPGM((lcd.getWidth()/2)-50, (lcd.getHeight()/2)-10, PSTR("Server"), RGB(0,0,0), RGB(255,255,255), 2);
  color = RGB(0,0,255);
#else
  lcd.drawTextPGM((lcd.getWidth()/2)-50, (lcd.getHeight()/2)-10, PSTR("Client"), RGB(0,0,0), RGB(255,255,255), 2);
  color = RGB(255,0,0);
#endif
  delay(1000);
  lcd.fillScreen(RGB(255,255,255));

  //start WiFi
  while(start_wifi() != 0){ delay(1000); }
}


void loop()
{
  uint16_t x=0, y=0;
  uint16_t buf[4];
  static uint8_t connection=0;

#if WORKING_MODE == 0 //server
  if(!server.connected())
  {
    if(server.connect() == 0) //open port for client (begin is the same as connect)
    {
      server.stop();
      if(++connection >= 3)
      {
        while(start_wifi() != 0){ delay(1000); }
      }
    }
    else
    {
      connection = 0;
    }
  }
  else
  {
    while(server.available() >= 8) //data available?
    {
      //0xBEEF, x, y, color (all 16bit, total 8 bytes)
      buf[0]  = (server.read()<<0); //1
      buf[0] |= (server.read()<<8);
      if(buf[0] == 0xBEEF)
      {
        buf[1]  = (server.read()<<0); //2
        buf[1] |= (server.read()<<8);
        buf[2]  = (server.read()<<0); //3
        buf[2] |= (server.read()<<8);
        buf[3]  = (server.read()<<0); //4
        buf[3] |= (server.read()<<8);
        if((buf[1] == 0) &&
           (buf[2] == 0) &&
           (buf[3] == 0))
        {
          clearall(0);
        }
        else
        {
          lcd.fillRect(buf[1], buf[2], 4, 4, buf[3]);
        }
      }
    }
  }
#else
  if(!client.connected())
  {
    if(client.connect() == 0) //connect to server
    {
      client.stop();
      if(++connection >= 3)
      {
        while(start_wifi() != 0){ delay(1000); }
      }
    }
    else
    {
      connection = 0;
    }
  }
  else
  {
    while(client.available() >= 8) //data available?
    {
      //0xBEEF, x, y, color (all 16bit, total 8 bytes)
      buf[0]  = (client.read()<<0); //1
      buf[0] |= (client.read()<<8);
      if(buf[0] == 0xBEEF)
      {
        buf[1]  = (client.read()<<0); //2
        buf[1] |= (client.read()<<8);
        buf[2]  = (client.read()<<0); //3
        buf[2] |= (client.read()<<8);
        buf[3]  = (client.read()<<0); //4
        buf[3] |= (client.read()<<8);
        if((buf[1] == 0) &&
           (buf[2] == 0) &&
           (buf[3] == 0))
        {
          clearall(0);
        }
        else
        {
          lcd.fillRect(buf[1], buf[2], 4, 4, buf[3]);
        }
      }
    }
  }
#endif

  //service routine for touch panel
  lcd.touchRead();

  //touch press?
  if(lcd.touchZ() &&
#if WORKING_MODE == 0 //server
     server.connected() && !server.available()) //server
#else
     client.connected() && !client.available()) //client
#endif
  {
    x = lcd.touchX();
    y = lcd.touchY();

    //change color
    if(x < 20)
    {
      last_x = 0;
      switch(color)
      {
        case RGB(0,0,0):   color = RGB(255,0,0); break;
        case RGB(255,0,0): color = RGB(0,255,0); break;
        case RGB(0,255,0): color = RGB(0,0,255); break;
        case RGB(0,0,255): color = RGB(0,0,0);   break;
      }
      lcd.fillRect(0, 0, 20, lcd.getHeight(), color);
      while(lcd.touchZ()){ lcd.touchRead(); }
    }

    //clear screen
    else if(x > (lcd.getWidth()-20))
    {
      clearall(1);
      while(lcd.touchZ()){ lcd.touchRead(); }
    }

    //draw line
    else if((last_x == 0) ||
            ((x >= (last_x-10)) && (x <= (last_x+10)) && (y >= (last_y-10)) && (y <= (last_y+10))))
    {
      if((x != last_x) || (y != last_y))
      {
        last_x = x;
        last_y = y;
        lcd.fillRect(x, y, 4, 4, color);

        //0xBEEF, x, y, color (all 16bit, total 8 bytes)
        buf[0] = 0xBEEF;
        buf[1] = x;
        buf[2] = y;
        buf[3] = color;
#if WORKING_MODE == 0 //server
        server.write((uint8_t*)buf, 8);
#else
        client.write((uint8_t*)buf, 8);
#endif
      }
    }
  }
  else
  {
    last_x = 0;
  }
}
