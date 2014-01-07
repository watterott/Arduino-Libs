/*
  Paint
 
  This sketch connects two RedFly-Shields with displays. 
  Requires RedFly + mSD-shield with MI0283QT-Adapter.
 */

#include <SPI.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <ADS7846.h>
#include <RedFly.h>
#include <RedFlyClient.h>
#include <RedFlyServer.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
MI0283QT9 lcd;  //MI0283QT9 Adapter v1

ADS7846 tp;

#define TP_EEPROMADDR (0x00) //eeprom address for calibration data

byte ip_server[] = { 192,168,  0, 50 }; //ip from server shield
byte ip_client[] = { 192,168,  0, 51 }; //ip from client shield

uint16_t comm_port = 1000; //communication port

RedFlyClient client(ip_server, comm_port);
RedFlyServer server(comm_port);

uint8_t working_mode=0; //0=server, 1=client

static uint16_t last_x=0, last_y=0, color=RGB(0,0,0);


//display output functions
#define infoClear()  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(255,255,255))
#define infoText(x)  lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,255,255), RGB(0,0,0), 1)
#define errorText(x) lcd.fillRect(0, (lcd.getHeight()/2)-15, lcd.getWidth(), 20, RGB(0,0,0)); lcd.drawTextPGM((lcd.getWidth()/2)-60, (lcd.getHeight()/2)-10, PSTR(x), RGB(255,0,0), RGB(0,0,0), 1)


uint8_t clearall(uint8_t send) //redraw complete screen
{
  if(send)
  {
    uint16_t buf[4]={0xBEEF,0x0000,0x0000,0x0000};
    if(working_mode == 0) //server
    {
      server.write((uint8_t*)buf, 8);
    }
    else
    {
      client.write((uint8_t*)buf, 8);
    }
  }

  lcd.fillRect(0, 0, 20, lcd.getHeight(), color);
  lcd.fillRect(lcd.getWidth()-20, 0, 20, lcd.getHeight(), RGB(0,0,0));
  lcd.fillRect(20, 0, lcd.getWidth()-40, lcd.getHeight(), RGB(255,255,255));
}


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

  //join/create network
  for(i=5; i!=0; i--) //try 5 times
  {
    if(working_mode == 0) //server
    {
      infoText("Create network...");
      ret = RedFly.join("PAINT", "1A2B3C4D5E", IBSS_CREATOR, 1); //create ad-hoc network
    }
    else
    {
      infoText("Scan...");
      RedFly.scan(); //scan for wireless networks (must be run before join command)
      infoText("Join...");
      ret = RedFly.join("PAINT", "1A2B3C4D5E", IBSS_JOINER); //join ad-hoc network
    }
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
  infoText("Set IP...");
  if(working_mode == 0) //server
  {
    ret = RedFly.begin(ip_server);
  }
  else
  {
    ret = RedFly.begin(ip_client);
  }
  if(ret)
  {
    errorText("Set IP...Error");
    RedFly.disconnect();
    return 3;
  }

  if(working_mode == 0) //server
  {
    infoText("Start Server...");
    server.begin();
  }
  else
  {
    infoText("Start Client...");
    client.connect();
  }

  clearall(1);

  return 0;
}


void setup()
{
  //init display
  lcd.begin(SPI_CLOCK_DIV2, 8); //SPI Displays: spi-clk=Fcpu/2, rst-pin=8
  lcd.fillScreen(RGB(255,255,255));

  //init touch controller
  tp.begin();

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
  lcd.fillScreen(RGB(255,255,255));

  //select working mode
  lcd.drawTextPGM(30, (lcd.getHeight()/2)-20, PSTR("Server"), RGB(0,0,0), RGB(255,255,255), 2);
  lcd.drawTextPGM((lcd.getWidth()/2)+30, (lcd.getHeight()/2)-20, PSTR("Client"), RGB(0,0,0), RGB(255,255,255), 2);
  lcd.fillRect((lcd.getWidth()/2)-1, 0, 2, lcd.getHeight(), RGB(0,0,0));
  while(tp.getPressure() < 5){ tp.service(); }
  if(tp.getX() < (lcd.getWidth()/2))
  {
    working_mode = 0; //server
    lcd.drawTextPGM(30, (lcd.getHeight()/2)-20, PSTR("Server"), RGB(255,0,0), RGB(255,255,255), 2);
    color = RGB(0,0,255);
  }
  else
  {
    working_mode = 1; //client
    lcd.drawTextPGM((lcd.getWidth()/2)+30, (lcd.getHeight()/2)-20, PSTR("Client"), RGB(255,0,0), RGB(255,255,255), 2);
    color = RGB(255,0,0);
  }
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

  if(working_mode == 0) //server
  {
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
  }
  else //client
  {
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
  }

  //service routine for touch panel
  tp.service();

  //touch press?
  if((tp.getPressure() > 10) &&
     (((working_mode == 0) && server.connected() && !server.available()) || //server
      ((working_mode == 1) && client.connected() && !client.available())))   //client
  {
    x = tp.getX();
    y = tp.getY();
 
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
      while(tp.getPressure() > 5){ tp.service(); }
    }

    //clear screen
    else if(x > (lcd.getWidth()-20))
    {
      clearall(1);
      while(tp.getPressure() > 5){ tp.service(); }
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
        if(working_mode == 0) //server
        {
          server.write((uint8_t*)buf, 8);
        }
        else
        {
          client.write((uint8_t*)buf, 8);
        }
      }
    }
  }
  else
  {
    last_x = 0;
  }
}
