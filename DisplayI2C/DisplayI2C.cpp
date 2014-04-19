/*
  MI0283QT-Adapter v2 + GLCD-Shield Lib for Arduino
  by Watterott electronic (www.watterott.com)
 */

#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
# include <avr/pgmspace.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "../Wire/Wire.h"
#include "../GraphicsLib/GraphicsLib.h"
#include "DisplayI2C.h"
#include "cmd.h"


#define I2C_ADDR 0x20


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define CS_PIN         (7)

#elif (defined(__AVR_ATmega644__) || \
       defined(__AVR_ATmega644P__))    //--- Arduino 644 (www.mafu-foto.de) ---

# define CS_PIN         (13)

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define CS_PIN         (7)

#else                                  //--- Arduino Uno ---

# define CS_PIN         (7)

#endif


//-------------------- Constructor --------------------


DisplayI2C::DisplayI2C(void) : GraphicsLib()
{
  return;
}


//-------------------- Public --------------------


void DisplayI2C::begin(uint_least8_t addr, uint_least8_t rst_pin)
{
  //init pins
  if(rst_pin < 0xFF)
  {
    pinMode(rst_pin, OUTPUT);
    digitalWrite(rst_pin, HIGH);
  }

  //reset display
  Wire.begin();

  bigger_8bit = 0;
  i2c_addr = addr;
  lcd_x = lcd_y = lcd_z = 0;

  if(getSize())
  {
    if(rst_pin < 0xFF)
    {
      pinMode(CS_PIN, OUTPUT);
      digitalWrite(CS_PIN, HIGH);
      digitalWrite(rst_pin, LOW);
      delay(10);
      digitalWrite(rst_pin, HIGH);
      digitalWrite(CS_PIN, LOW);
      delay(20);
      digitalWrite(CS_PIN, HIGH);
      
      if(getSize()) //older displays need about 800ms
      {
        digitalWrite(rst_pin, LOW);
        delay(10);
        digitalWrite(rst_pin, HIGH);
        digitalWrite(CS_PIN, LOW);
        delay(800);
        digitalWrite(CS_PIN, HIGH);
      }
    }
    getSize();
  }

  setOrientation(0); //display rotation
  fillScreen(0); //clear display buffer
  led(50); //enable backlight
  features = getFeatures();
  if(features & FEATURE_TP)
  {
    setFeatures(FEATURE_TP); //enable touchpanel
  }

  delay(80); //wait till all commands are progressed

  return;
}


void DisplayI2C::begin(uint_least8_t addr)
{
  return begin(addr, 0xFF);
}


void DisplayI2C::begin(void)
{
  return begin(I2C_ADDR, 0xFF);
}


uint_least8_t DisplayI2C::getSize(void)
{
  uint_least16_t w=0, h=0;
  
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_WIDTH);
  Wire.endTransmission();
  Wire.requestFrom(i2c_addr, 2); //request 2 bytes
  delay(1);
  if(Wire.available())
  { 
    w  = Wire.read()<<8;
    w |= Wire.read()<<0;
  }

  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_HEIGHT);
  Wire.endTransmission();
  Wire.requestFrom(i2c_addr, 2); //request 2 bytes
  delay(1);
  if(Wire.available())
  { 
    h  = Wire.read()<<8;
    h |= Wire.read()<<0;
  }

  if((w < 10) || (w > 1000) ||
     (h < 10) || (h > 1000))
  {
    return 1;
  }

  lcd_width  = w;
  lcd_height = h;

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


uint_least8_t DisplayI2C::getVersion(char *v)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_VERSION);
  Wire.endTransmission();
  Wire.requestFrom(i2c_addr, 4); //request 4 bytes
  delay(1);
  if(Wire.available())
  {
    v[0] = (char)Wire.read();
    v[1] = (char)Wire.read();
    v[2] = (char)Wire.read();
    v[3] = (char)Wire.read();
    v[4] = 0;
  }

  if(v[1] != '.')
  {
    return 1;
  }

  return 0;
}


uint_least8_t DisplayI2C::getFeatures(void)
{
  uint_least8_t f=0;

  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_FEATURES);
  Wire.endTransmission();
  Wire.requestFrom(i2c_addr, 1); //request 1 bytes
  delay(1);
  if(Wire.available())
  {
    f = Wire.read();
  }

  return f;
}


void DisplayI2C::setFeatures(uint_least8_t f)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_CTRL);
  Wire.write(CMD_CTRL_FEATURES);
  Wire.write(f);
  Wire.endTransmission();
  delay(10); //wait to set features

  return;
}


void DisplayI2C::led(uint_least8_t power)
{
  if(power <= 100)
  {
    Wire.beginTransmission(i2c_addr);
    Wire.write(CMD_LCD_LED);
    Wire.write(power); //0...100
    Wire.endTransmission();
  }

  return;
}


void DisplayI2C::invertDisplay(uint_least8_t invert)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_INVERT);
  Wire.write(invert);
  Wire.endTransmission();

  return;
}


void DisplayI2C::setOrientation(uint_least16_t o)
{
  uint_least16_t w=lcd_width, h=lcd_height;

  switch(o)
  {
    default:
    case 0:
      lcd_orientation = 0;
      lcd_width  = w;
      lcd_height = h;
      break;

    case  9:
    case 90:
      lcd_orientation = 90;
      lcd_width  = h;
      lcd_height = w;
      break;

    case  18:
    case 180:
      lcd_orientation = 180;
      lcd_width  = w;
      lcd_height = h;
      break;

    case  27:
    case  14: //270&0xFF
    case 270:
      lcd_orientation = 270;
      lcd_width  = h;
      lcd_height = w;
      break;
  }

  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_ORIENTATION);
  Wire.write(lcd_orientation/10); //0, 9, 18, 27
  Wire.endTransmission();

  return;
}


void DisplayI2C::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  return;
}


void DisplayI2C::drawStart(void)
{
  return;
}


void DisplayI2C::draw(uint_least16_t color)
{
  return;
}


void DisplayI2C::drawStop(void)
{
  return;
}


void DisplayI2C::fillScreen(uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_CLEAR);
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWPIXEL);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
  }
  else
  {
    Wire.write(x0>>0);
    Wire.write(y0>>0);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWLINE);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(x1>>8);
    Wire.write(x1>>0);
    Wire.write(y1>>8);
    Wire.write(y1>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(x1);
    Wire.write(y1);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWRECT);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(w>>8);
    Wire.write(w>>0);
    Wire.write(h>>8);
    Wire.write(h>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(w);
    Wire.write(h);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_FILLRECT);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(w>>8);
    Wire.write(w>>0);
    Wire.write(h>>8);
    Wire.write(h>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(w);
    Wire.write(h);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);

  return;
}


void DisplayI2C::fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
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


void DisplayI2C::drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWRNDRECT);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(w>>8);
    Wire.write(w>>0);
    Wire.write(h>>8);
    Wire.write(h>>0);
    Wire.write(r>>8);
    Wire.write(r>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(w);
    Wire.write(h);
    Wire.write(r);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_FILLRNDRECT);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(w>>8);
    Wire.write(w>>0);
    Wire.write(h>>8);
    Wire.write(h>>0);
    Wire.write(r>>8);
    Wire.write(r>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(w);
    Wire.write(h);
    Wire.write(r);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWCIRCLE);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(r>>8);
    Wire.write(r>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(r);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_FILLCIRCLE);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(r>>8);
    Wire.write(r>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(r);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWELLIPSE);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(r_x>>8);
    Wire.write(r_x>>0);
    Wire.write(r_y>>8);
    Wire.write(r_y>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(r_x);
    Wire.write(r_y);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


void DisplayI2C::fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_FILLELLIPSE);
  if(bigger_8bit)
  {
    Wire.write(x0>>8);
    Wire.write(x0>>0);
    Wire.write(y0>>8);
    Wire.write(y0>>0);
    Wire.write(r_x>>8);
    Wire.write(r_x>>0);
    Wire.write(r_y>>8);
    Wire.write(r_y>>0);
  }
  else
  {
    Wire.write(x0);
    Wire.write(y0);
    Wire.write(r_x);
    Wire.write(r_y);
  }
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.endTransmission();

  return;
}


int_least16_t DisplayI2C::drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  static uint_least32_t last_time=0;

  while(millis() == last_time); //slow down function calling
  last_time = millis();

  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_LCD_DRAWTEXT);
  Wire.write(color>>8);
  Wire.write(color>>0);
  Wire.write(bg>>8);
  Wire.write(bg>>0);
  if(bigger_8bit)
  {
    Wire.write(x>>8);
    Wire.write(x>>0);
    Wire.write(y>>8);
    Wire.write(y>>0);
  }
  else
  {
    Wire.write(x);
    Wire.write(y);
  }
  Wire.write(size);
  Wire.write(1);
  Wire.write(c);
  Wire.endTransmission();

  return x+(8*(size&0x7F));
}


int_least16_t DisplayI2C::drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  return drawChar(x, y, (char)c, color, bg, size);
}


int_least16_t DisplayI2C::drawText(int_least16_t x, int_least16_t y, char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;

  len = strlen(s);

  if((len != 0) && (len < 256))
  {
    Wire.beginTransmission(i2c_addr);
    Wire.write(CMD_LCD_DRAWTEXT);
    Wire.write(color>>8);
    Wire.write(color>>0);
    Wire.write(bg>>8);
    Wire.write(bg>>0);
    if(bigger_8bit)
    {
      Wire.write(x>>8);
      Wire.write(x>>0);
      Wire.write(y>>8);
      Wire.write(y>>0);
    }
    else
    {
      Wire.write(x);
      Wire.write(y);
    }
    Wire.write(size);
    Wire.write(len);
    Wire.endTransmission();
    size = 8*(size&0x7F);
    while(*s != 0)
    {
      Wire.beginTransmission(i2c_addr);
      Wire.write((uint8_t)*s++);
      Wire.endTransmission();
      pos += size;
    }
  }

  return pos;
}


int_least16_t DisplayI2C::drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  int_least16_t pos=x;

  if((s.length() != 0) && (s.length() < 256))
  {
    Wire.beginTransmission(i2c_addr);
    Wire.write(CMD_LCD_DRAWTEXT);
    Wire.write(color>>8);
    Wire.write(color>>0);
    Wire.write(bg>>8);
    Wire.write(bg>>0);
    if(bigger_8bit)
    {
      Wire.write(x>>8);
      Wire.write(x>>0);
      Wire.write(y>>8);
      Wire.write(y>>0);
    }
    else
    {
      Wire.write(x);
      Wire.write(y);
    }
    Wire.write(size);
    Wire.write((uint8_t)s.length());
    Wire.endTransmission();
    size = 8*(size&0x7F);
    for(uint_least16_t i=0; i < s.length(); i++) 
    {
      Wire.beginTransmission(i2c_addr);
      Wire.write((uint8_t)s[i]);
      Wire.endTransmission();
      pos += size;
    }
  }

  return pos;
}


#if defined(__AVR__)
int_least16_t DisplayI2C::drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;
  uint_least8_t c;

  len = strlen_P(s);

  if((len != 0) && (len < 256))
  {
    Wire.beginTransmission(i2c_addr);
    Wire.write(CMD_LCD_DRAWTEXT);
    Wire.write(color>>8);
    Wire.write(color>>0);
    Wire.write(bg>>8);
    Wire.write(bg>>0);
    if(bigger_8bit)
    {
      Wire.write(x>>8);
      Wire.write(x>>0);
      Wire.write(y>>8);
      Wire.write(y>>0);
    }
    else
    {
      Wire.write(x);
      Wire.write(y);
    }
    Wire.write(size);
    Wire.write(len);
    Wire.endTransmission();
    size = 8*(size&0x7F);
    c = pgm_read_byte(s++);
    while(c != 0)
    {
      Wire.beginTransmission(i2c_addr);
      Wire.write(c);
      Wire.endTransmission();
      c = pgm_read_byte(s++);
      pos += size;
    }
  }

  return pos;
}
#endif


uint_least8_t DisplayI2C::touchRead(void)
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
    
    Wire.flush();
    Wire.beginTransmission(i2c_addr);
    Wire.write(CMD_TP_POS);
    Wire.endTransmission();

    if(bigger_8bit)
    {
      Wire.requestFrom(i2c_addr, 6);
      while(Wire.available() == 0);
      lcd_x  = Wire.read()<<8;
      lcd_x |= Wire.read()<<0;
      lcd_y  = Wire.read()<<8;
      lcd_y |= Wire.read()<<0;
      lcd_z  = Wire.read()<<8;
      lcd_z |= Wire.read()<<0;
    }
    else
    {
      Wire.requestFrom(i2c_addr, 3);
      while(Wire.available() == 0);
      lcd_x = Wire.read();
      lcd_y = Wire.read();
      lcd_z = Wire.read();
    }

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


void DisplayI2C::touchStartCal(void)
{
  uint_least32_t ms, last_time, timeout;

  if(!(features & FEATURE_TP))
  {
    return;
  }

  Wire.beginTransmission(i2c_addr);
  Wire.write(CMD_TP_CALIBRATE);
  Wire.endTransmission();

  for(last_time=0, timeout=0; 1;)
  { 
    ms = millis();
    if((ms-last_time) > 1000) //check every 1s
    {
      last_time = ms; //save time
      if(++timeout >= 30) //timeout after 30s
      {
        Wire.beginTransmission(i2c_addr);
        Wire.write((byte)0x00);
        Wire.endTransmission();
        break;
      }
      Wire.requestFrom(i2c_addr, 1); //request 1 bytes
    }
    if(Wire.available())
    {
      if(Wire.read() == CMD_TP_CALIBRATE)
      {
        break;
      }
    }
  }
  delay(200);
  Wire.flush();
  while(Wire.available()){ Wire.read(); } //read response
  delay(20);

  return;
}


int_least16_t DisplayI2C::touchX(void)
{
  return lcd_x;
}


int_least16_t DisplayI2C::touchY(void)
{
  return lcd_y;
}


int_least16_t DisplayI2C::touchZ(void)
{
  return lcd_z;
}
