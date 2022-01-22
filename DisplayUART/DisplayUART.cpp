/*
  MI0283QT-Adapter v2 + GLCD-Shield Lib for Arduino
  by Watterott electronic (www.watterott.com)
 */

#include <inttypes.h>
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
# include <avr/io.h>
# include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
# include <pgmspace.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "SoftwareSerial.h"
#include "Wire.h"
#include "digitalWriteFast.h"
#include "GraphicsLib.h"
#include "DisplayUART.h"
#include "cmd.h"


//#define I2C_ADDR        (0x20)

#define BAUDRATE        (9600)

#define RX_PIN          (10)
#define TX_PIN          (11)

SoftwareSerial SerialLCD(RX_PIN, TX_PIN);


//-------------------- Constructor --------------------


DisplayUART::DisplayUART(void) : GraphicsLib()
{
  return;
}


//-------------------- Public --------------------


void DisplayUART::begin(uint_least8_t rx_pin, uint_least8_t tx_pin, uint_least8_t cs_pin, uint_least8_t rst_pin)
{
  uint_least8_t err;

  //init pins
  if(rst_pin < 0xFF)
  {
    digitalWrite(rst_pin, HIGH);
    pinMode(rst_pin, OUTPUT);
  }
  pinMode(rx_pin, INPUT);
  digitalWrite(tx_pin, HIGH);
  pinMode(tx_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);
  pinMode(cs_pin, OUTPUT);
  pin_cs = cs_pin;

  //init serial lib
  if((RX_PIN != rx_pin) || (TX_PIN != tx_pin))
  {
    SerialLCD = SoftwareSerial(rx_pin, tx_pin);
  }
  SerialLCD.begin(BAUDRATE);

  //sync to display
  delay(20); //wait for start-up
  digitalWrite(cs_pin, LOW);
  SerialLCD.write((byte)0x00); //NOP and stop calibration
  SerialLCD.write((byte)0x00); //NOP and stop calibration
  SerialLCD.write((byte)0x00); //NOP and stop calibration
  digitalWrite(cs_pin, HIGH);

  //reset display
  bigger_8bit = 0;
  lcd_x = lcd_y = lcd_z = 0;
  err = getSize();
  if(err)
  {
    if(rst_pin < 0xFF)
    {
      digitalWrite(tx_pin, LOW); //CS+TX low -> UART
      digitalWrite(cs_pin, LOW);
      digitalWrite(rst_pin, LOW);
      delay(10);
      digitalWrite(rst_pin, HIGH);
      delay(20);
      digitalWrite(cs_pin, HIGH);
      delay(20);

      err = getSize();
      if(err) //older displays need about 800ms
      {
        digitalWrite(tx_pin, LOW); //CS+TX low -> UART
        digitalWrite(cs_pin, LOW);
        digitalWrite(rst_pin, LOW);
        delay(10);
        digitalWrite(rst_pin, HIGH);
        delay(800);
        digitalWrite(cs_pin, HIGH);
        delay(20);
        err = getSize();
      }
    }

    if(err)
    {
      #ifdef I2C_ADDR
        //set interface through default interface (I2C)
        Wire.begin();
        Wire.beginTransmission(I2C_ADDR);
        Wire.write(CMD_CTRL);
        Wire.write(CMD_CTRL_INTERFACE);
        Wire.write(INTERFACE_UART);
        Wire.endTransmission();
      #endif
      err = getSize();
    }
  }

  if(err == 0)
  {
    features = getFeatures();
    if(features & FEATURE_TP)
    {
      setFeatures(FEATURE_TP); //enable touchpanel
    }
    setOrientation(0); //display rotation
    fillScreen(0); //clear display buffer
    led(50); //enable backlight
    delay(100); //wait till all commands are progressed
  }

  return;
}


void DisplayUART::begin(uint_least8_t rx_pin, uint_least8_t tx_pin, uint_least8_t cs_pin)
{
  return begin(rx_pin, tx_pin, cs_pin, 0xFF);
}


uint_least8_t DisplayUART::getSize(void)
{
  uint_least16_t w=0, h=0;

  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_WIDTH);
  for(uint_least8_t timeout=50; timeout!=0; timeout--) //wait for data
  {
    if(SerialLCD.available() >= 2)
    {
      break;
    }
    delay(1);
  }
  w  = SerialLCD.read()<<8;
  w |= SerialLCD.read()<<0;

  SerialLCD.write(CMD_LCD_HEIGHT);
  for(uint_least8_t timeout=50; timeout!=0; timeout--) //wait for data
  {
    if(SerialLCD.available() >= 2)
    {
      break;
    }
    delay(1);
  }
  h  = SerialLCD.read()<<8;
  h |= SerialLCD.read()<<0;
  
  digitalWrite(pin_cs, HIGH);

  if((w < 10) || (w > 1000) ||
     (h < 10) || (h > 1000))
  {
    return 1;
  }

  lcd_width  = display_width = w;
  lcd_height = display_height = h;

  if((w > 255) || (h > 255))
  {
    bigger_8bit = 1;
  }
  else
  {
    bigger_8bit = 0;
  }

  return 0;
}


uint_least8_t DisplayUART::getVersion(char *v)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_VERSION);
  while(SerialLCD.available() < 4); //wait for data
  v[0] = (char)SerialLCD.read();
  v[1] = (char)SerialLCD.read();
  v[2] = (char)SerialLCD.read();
  v[3] = (char)SerialLCD.read();
  v[4] = 0;
  digitalWrite(pin_cs, HIGH);

  if(v[1] != '.')
  {
    return 1;
  }

  return 0;
}


uint_least8_t DisplayUART::getFeatures(void)
{
  uint_least8_t f=0;

  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_FEATURES);
  while(SerialLCD.available() < 1); //wait for data
  f = SerialLCD.read();
  digitalWrite(pin_cs, HIGH);

  return f;
}


void DisplayUART::setFeatures(uint_least8_t f)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_CTRL);
  SerialLCD.write(CMD_CTRL_FEATURES);
  SerialLCD.write(f);
  digitalWrite(pin_cs, HIGH);
  delay(10); //wait to set features

  return;
}


void DisplayUART::led(uint_least8_t power)
{
  if(power <= 100)
  {
    digitalWrite(pin_cs, LOW);
    SerialLCD.write(CMD_LCD_LED);
    SerialLCD.write(power); //0...100
    digitalWrite(pin_cs, HIGH);
  }

  return;
}


void DisplayUART::invertDisplay(uint_least8_t invert)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_INVERT);
  SerialLCD.write(invert);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::setOrientation(uint_least16_t o)
{
  switch(o)
  {
    default:
    case 0:
      lcd_orientation = 0;
      lcd_width  = display_width;
      lcd_height = display_height;
      break;

    case  9:
    case 90:
      lcd_orientation = 90;
      lcd_width  = display_height;
      lcd_height = display_width;
      break;

    case  18:
    case 180:
      lcd_orientation = 180;
      lcd_width  = display_width;
      lcd_height = display_height;
      break;

    case  27:
    case  14: //270&0xFF
    case 270:
      lcd_orientation = 270;
      lcd_width  = display_height;
      lcd_height = display_width;
      break;
  }

  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_ORIENTATION);
  SerialLCD.write(lcd_orientation/10); //0, 9, 18, 27
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  return;
}


void DisplayUART::drawStart(void)
{
  return;
}


void DisplayUART::draw(uint_least16_t color)
{
  return;
}


void DisplayUART::drawStop(void)
{
  return;
}


void DisplayUART::fillScreen(uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_CLEAR);
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWPIXEL);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
  }
  else
  {
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>0);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWLINE);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(x1>>8);
    SerialLCD.write(x1>>0);
    SerialLCD.write(y1>>8);
    SerialLCD.write(y1>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(x1);
    SerialLCD.write(y1);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWRECT);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(w>>8);
    SerialLCD.write(w>>0);
    SerialLCD.write(h>>8);
    SerialLCD.write(h>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(w);
    SerialLCD.write(h);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_FILLRECT);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(w>>8);
    SerialLCD.write(w>>0);
    SerialLCD.write(h>>8);
    SerialLCD.write(h>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(w);
    SerialLCD.write(h);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);

  return;
}


void DisplayUART::fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  int_least16_t a, b, y, last, sa, sb;
  int_least16_t dx01, dy01, dx02, dy02, dx12, dy12;

  //sort coordinates (y2 >= y1 >= y0)
  if(y0 > y1){ SWAP(y0, y1); SWAP(x0, x1); }
  if(y1 > y2){ SWAP(y2, y1); SWAP(x2, x1); }
  if(y0 > y1){ SWAP(y0, y1); SWAP(x0, x1); }

  if(y0 == y2) //all on the same line
  {
    a = b = x0;
         if(x1 < a){ a = x1; }
    else if(x1 > b){ b = x1; }
         if(x2 < a){ a = x2; }
    else if(x2 > b){ b = x2; }
    drawLine(a, y0, b, y0, color);
    return;
  }

  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1;

  //upper part of triangle
  if(y1 == y2) last = y1;
  else         last = y1-1;

  for(y=y0, sa=0, sb=0; y<=last; y++)
  {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    drawLine(a, y, b, y, color);
  }

  //lower part of triangle
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++)
  {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    drawLine(a, y, b, y, color);
  }

  return;
}


void DisplayUART::drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWRNDRECT);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(w>>8);
    SerialLCD.write(w>>0);
    SerialLCD.write(h>>8);
    SerialLCD.write(h>>0);
    SerialLCD.write(r>>8);
    SerialLCD.write(r>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(w);
    SerialLCD.write(h);
    SerialLCD.write(r);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_FILLRNDRECT);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(w>>8);
    SerialLCD.write(w>>0);
    SerialLCD.write(h>>8);
    SerialLCD.write(h>>0);
    SerialLCD.write(r>>8);
    SerialLCD.write(r>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(w);
    SerialLCD.write(h);
    SerialLCD.write(r);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWCIRCLE);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(r>>8);
    SerialLCD.write(r>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(r);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_FILLCIRCLE);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(r>>8);
    SerialLCD.write(r>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(r);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWELLIPSE);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(r_x>>8);
    SerialLCD.write(r_x>>0);
    SerialLCD.write(r_y>>8);
    SerialLCD.write(r_y>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(r_x);
    SerialLCD.write(r_y);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


void DisplayUART::fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_FILLELLIPSE);
  if(bigger_8bit)
  {
    SerialLCD.write(x0>>8);
    SerialLCD.write(x0>>0);
    SerialLCD.write(y0>>8);
    SerialLCD.write(y0>>0);
    SerialLCD.write(r_x>>8);
    SerialLCD.write(r_x>>0);
    SerialLCD.write(r_y>>8);
    SerialLCD.write(r_y>>0);
  }
  else
  {
    SerialLCD.write(x0);
    SerialLCD.write(y0);
    SerialLCD.write(r_x);
    SerialLCD.write(r_y);
  }
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  digitalWrite(pin_cs, HIGH);

  return;
}


int_least16_t DisplayUART::drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  static uint_least32_t last_time=0;

  while(millis() == last_time); //slow down function calling
  last_time = millis();

  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_LCD_DRAWTEXT);
  SerialLCD.write(color>>8);
  SerialLCD.write(color>>0);
  SerialLCD.write(bg>>8);
  SerialLCD.write(bg>>0);
  if(bigger_8bit)
  {
    SerialLCD.write(x>>8);
    SerialLCD.write(x>>0);
    SerialLCD.write(y>>8);
    SerialLCD.write(y>>0);
  }
  else
  {
    SerialLCD.write(x);
    SerialLCD.write(y);
  }
  SerialLCD.write(size);
  SerialLCD.write(1);
  SerialLCD.write(c);
  digitalWrite(pin_cs, HIGH);

  return x+(8*(size&0x7F));
}


int_least16_t DisplayUART::drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  return drawChar(x, y, (char)c, color, bg, size);
}


int_least16_t DisplayUART::drawText(int_least16_t x, int_least16_t y, const char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;

  len = strlen(s);

  if((len != 0) && (len < 256))
  {
    digitalWrite(pin_cs, LOW);
    SerialLCD.write(CMD_LCD_DRAWTEXT);
    SerialLCD.write(color>>8);
    SerialLCD.write(color>>0);
    SerialLCD.write(bg>>8);
    SerialLCD.write(bg>>0);
    if(bigger_8bit)
    {
      SerialLCD.write(x>>8);
      SerialLCD.write(x>>0);
      SerialLCD.write(y>>8);
      SerialLCD.write(y>>0);
    }
    else
    {
      SerialLCD.write(x);
      SerialLCD.write(y);
    }
    SerialLCD.write(size);
    SerialLCD.write(len);
    size = 8*(size&0x7F);
    while(*s != 0)
    {
      SerialLCD.write((uint8_t)*s++);
      pos += size;
    }
    digitalWrite(pin_cs, HIGH);
  }

  return pos;
}


int_least16_t DisplayUART::drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  int_least16_t pos=x;

  if((s.length() != 0) && (s.length() < 256))
  {
    digitalWrite(pin_cs, LOW);
    SerialLCD.write(CMD_LCD_DRAWTEXT);
    SerialLCD.write(color>>8);
    SerialLCD.write(color>>0);
    SerialLCD.write(bg>>8);
    SerialLCD.write(bg>>0);
    if(bigger_8bit)
    {
      SerialLCD.write(x>>8);
      SerialLCD.write(x>>0);
      SerialLCD.write(y>>8);
      SerialLCD.write(y>>0);
    }
    else
    {
      SerialLCD.write(x);
      SerialLCD.write(y);
    }
    SerialLCD.write(size);
    SerialLCD.write((uint8_t)s.length());
    size = 8*(size&0x7F);
    for(uint_least16_t i=0; i < s.length(); i++) 
    {
      SerialLCD.write((uint8_t)s[i]);
      pos += size;
    }
    digitalWrite(pin_cs, HIGH);
  }

  return pos;
}


#if defined(__AVR__)
int_least16_t DisplayUART::drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;
  uint_least8_t c;

  len = strlen_P(s);

  if((len != 0) && (len < 256))
  {
    digitalWrite(pin_cs, LOW);
    SerialLCD.write(CMD_LCD_DRAWTEXT);
    SerialLCD.write(color>>8);
    SerialLCD.write(color>>0);
    SerialLCD.write(bg>>8);
    SerialLCD.write(bg>>0);
    if(bigger_8bit)
    {
      SerialLCD.write(x>>8);
      SerialLCD.write(x>>0);
      SerialLCD.write(y>>8);
      SerialLCD.write(y>>0);
    }
    else
    {
      SerialLCD.write(x);
      SerialLCD.write(y);
    }
    SerialLCD.write(size);
    SerialLCD.write(len);
    size = 8*(size&0x7F);
    c = pgm_read_byte(s++);
    while(c != 0)
    {
      SerialLCD.write(c);
      c = pgm_read_byte(s++);
      pos += size;
    }
    digitalWrite(pin_cs, HIGH);
  }

  return pos;
}
#endif


uint_least8_t DisplayUART::touchRead(void)
{
  uint_least32_t ms;
  static uint_least32_t last_time=0;

  if(!(features & FEATURE_TP))
  {
    return 0;
  }

  ms = millis();
  if((ms-last_time) > 10) //can only be read every 10ms
  {
    last_time = ms; //save time

    digitalWrite(pin_cs, LOW);
    SerialLCD.flush();
    SerialLCD.write(CMD_TP_POS);

    if(bigger_8bit)
    {
      for(uint_least8_t timeout=100; timeout!=0; timeout--) //wait for data
      {
        if(SerialLCD.available() >= 6)
        {
          break;
        }
        delayMicroseconds(100);
      }
      lcd_x  = SerialLCD.read()<<8;
      lcd_x |= SerialLCD.read()<<0;
      lcd_y  = SerialLCD.read()<<8;
      lcd_y |= SerialLCD.read()<<0;
      lcd_z  = SerialLCD.read()<<8;
      lcd_z |= SerialLCD.read()<<0;
    }
    else
    {
      for(uint_least8_t timeout=100; timeout!=0; timeout--) //wait for data
      {
        if(SerialLCD.available() >= 3)
        {
          break;
        }
        delayMicroseconds(100);
      }
      lcd_x = SerialLCD.read();
      lcd_y = SerialLCD.read();
      lcd_z = SerialLCD.read();
    }
    digitalWrite(pin_cs, HIGH);

    if((lcd_z == 0xFFFF) || (lcd_x < 0) || (lcd_y < 0))
    {
      lcd_z = 0;
    }
  }

  if(lcd_z)
  {
    return 1;
  }

  return 0;
}


void DisplayUART::touchStartCal(void)
{
  uint_least32_t ms, last_time, timeout;

  if(!(features & FEATURE_TP))
  {
    return;
  }

  digitalWrite(pin_cs, LOW);
  SerialLCD.write(CMD_TP_CALIBRATE);

  for(last_time=0, timeout=0; 1;)
  { 
    ms = millis();
    if((ms-last_time) > 1000) //check every 1s
    {
      last_time = ms; //save time
      if(++timeout >= 30) //timeout after 30s
      {
        SerialLCD.write((byte)0x00);
        break;
      }
    }
    if(SerialLCD.available())
    {
      if(SerialLCD.read() == CMD_TP_CALIBRATE)
      {
        break;
      }
    }
  }
  delay(200);
  SerialLCD.flush();
  while(SerialLCD.available()){ SerialLCD.read(); } //read response
  delay(20);
  digitalWrite(pin_cs, HIGH);

  return;
}


int_least16_t DisplayUART::touchX(void)
{
  return lcd_x;
}


int_least16_t DisplayUART::touchY(void)
{
  return lcd_y;
}


int_least16_t DisplayUART::touchZ(void)
{
  return lcd_z;
}
