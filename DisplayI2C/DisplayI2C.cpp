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

  i2c_addr = addr;
  Wire.begin();

  //reset display
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
  
  bigger_8bit = 0;

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
