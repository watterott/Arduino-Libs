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
#include "SPI.h"
#include "Wire.h"
#include "digitalWriteFast.h"
#include "GraphicsLib.h"
#include "DisplaySPI.h"
#include "cmd.h"


#define I2C_ADDR 0x20


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define CS_PIN         (7)
# define MOSI_PIN      (51)
# define MISO_PIN      (50)
# define CLK_PIN       (52)

#elif (defined(__AVR_ATmega644__) || \
       defined(__AVR_ATmega644P__))    //--- Arduino 644 (www.mafu-foto.de) ---

# define CS_PIN         (13)
# define MOSI_PIN       (5)
# define MISO_PIN       (6)
# define CLK_PIN        (7)

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define CS_PIN         (7)
# define MOSI_PIN       (16) //PB2
# define MISO_PIN       (14) //PB3
# define CLK_PIN        (15) //PB1

#else                                  //--- Arduino Uno ---

# define CS_PIN         (7)
# define MOSI_PIN       (11)
# define MISO_PIN       (12)
# define CLK_PIN        (13)

#endif


#define CS_DISABLE()    digitalWriteFast(CS_PIN, HIGH)
#define CS_ENABLE()     digitalWriteFast(CS_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define MISO_READ()     digitalReadFast(MISO_PIN)

#define CLK_HIGH()      digitalWriteFast(CLK_PIN, HIGH)
#define CLK_LOW()       digitalWriteFast(CLK_PIN, LOW)


//-------------------- Constructor --------------------


DisplaySPI::DisplaySPI(void) : GraphicsLib()
{
  return;
}


//-------------------- Public --------------------


void DisplaySPI::begin(uint_least8_t clock_div, uint_least8_t rst_pin)
{
  //init pins
  if(rst_pin < 0xFF)
  {
    pinMode(rst_pin, OUTPUT);
    digitalWrite(rst_pin, HIGH);
  }
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);

  //reset display
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(clock_div);
  SPI.begin();

  bigger_8bit = 0;
  lcd_x = lcd_y = lcd_z = 0;
  
  if(getSize())
  {
    if(rst_pin < 0xFF)
    {
      SPI.end();
      digitalWrite(rst_pin, LOW);
      delay(10);
      digitalWrite(rst_pin, HIGH);
      digitalWrite(MOSI_PIN, LOW); //CS+MOSI low -> SPI
      digitalWrite(CS_PIN, LOW);
      delay(20);
      digitalWrite(CS_PIN, HIGH);
      SPI.begin();
      delay(10);

      if(getSize()) //older displays need about 800ms
      {
        SPI.end();
        digitalWrite(rst_pin, LOW);
        delay(10);
        digitalWrite(rst_pin, HIGH);
        digitalWrite(MOSI_PIN, LOW); //CS+MOSI low -> SPI
        digitalWrite(CS_PIN, LOW);
        delay(800);
        digitalWrite(CS_PIN, HIGH);
        SPI.begin();
        delay(10);
      }
    }

    if(getSize())
    {
      //set interface through default interface (I2C)
      Wire.begin();
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CMD_CTRL);
      Wire.write(CMD_CTRL_INTERFACE);
      Wire.write(INTERFACE_SPI);
      Wire.endTransmission();
      getSize();
    }
  }

  features = getFeatures();
  if(features & FEATURE_TP)
  {
    setFeatures(FEATURE_TP); //enable touchpanel
  }
  setOrientation(0); //display rotation
  fillScreen(0); //clear display buffer
  led(50); //enable backlight

  delay(100); //wait till all commands are progressed

  return;
}


void DisplaySPI::begin(uint_least8_t clock_div)
{
  return begin(clock_div, 0xFF);
}


void DisplaySPI::begin(void)
{
#if F_CPU > 32000000UL
  return begin(SPI_CLOCK_DIV16, 0xFF);
#elif F_CPU > 16000000UL
  return begin(SPI_CLOCK_DIV8, 0xFF);
#else //if F_CPU > 8000000UL
  return begin(SPI_CLOCK_DIV4, 0xFF);
#endif
}


uint_least8_t DisplaySPI::getSize(void)
{
  uint_least16_t w=0, h=0;

  bigger_8bit = 0;

  CS_ENABLE();
  SPI.transfer(CMD_LCD_WIDTH);
  delay(1);
  w  = SPI.transfer(0xFF)<<8;
  w |= SPI.transfer(0xFF)<<0;
  SPI.transfer(CMD_LCD_HEIGHT);
  delay(1);
  h  = SPI.transfer(0xFF)<<8;
  h |= SPI.transfer(0xFF)<<0;
  CS_DISABLE();

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


uint_least8_t DisplaySPI::getVersion(char *v)
{
  CS_ENABLE();
  SPI.transfer(CMD_VERSION);
  delay(1);
  v[0] = (char)SPI.transfer(0xFF);
  v[1] = (char)SPI.transfer(0xFF);
  v[2] = (char)SPI.transfer(0xFF);
  v[3] = (char)SPI.transfer(0xFF);
  v[4] = 0;
  CS_DISABLE();

  if(v[1] != '.')
  {
    return 1;
  }

  return 0;
}


uint_least8_t DisplaySPI::getFeatures(void)
{
  uint_least8_t f=0;

  CS_ENABLE();
  SPI.transfer(CMD_FEATURES);
  delay(1);
  f = SPI.transfer(0xFF);
  CS_DISABLE();

  return f;
}


void DisplaySPI::setFeatures(uint_least8_t f)
{
  CS_ENABLE();
  SPI.transfer(CMD_CTRL);
  SPI.transfer(CMD_CTRL_FEATURES);
  SPI.transfer(f);
  CS_DISABLE();
  delay(10); //wait to set features

  return;
}


void DisplaySPI::led(uint_least8_t power)
{
  if(power <= 100)
  {
    CS_ENABLE();
    SPI.transfer(CMD_LCD_LED);
    SPI.transfer(power); //0...100
    CS_DISABLE();
  }

  return;
}


void DisplaySPI::invertDisplay(uint_least8_t invert)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_INVERT);
  SPI.transfer(invert);
  CS_DISABLE();

  return;
}


void DisplaySPI::setOrientation(uint_least16_t o)
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

  CS_ENABLE();
  SPI.transfer(CMD_LCD_ORIENTATION);
  SPI.transfer(lcd_orientation/10); //0, 9, 18, 27
  CS_DISABLE();

  return;
}

void DisplaySPI::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  return;
}


void DisplaySPI::drawStart(void)
{
  return;
}


void DisplaySPI::draw(uint_least16_t color)
{
  return;
}


void DisplaySPI::drawStop(void)
{
  return;
}


void DisplaySPI::fillScreen(uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_CLEAR);
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWPIXEL);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
  }
  else
  {
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>0);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWLINE);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(x1>>8);
    SPI.transfer(x1>>0);
    SPI.transfer(y1>>8);
    SPI.transfer(y1>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(x1);
    SPI.transfer(y1);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWRECT);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(w>>8);
    SPI.transfer(w>>0);
    SPI.transfer(h>>8);
    SPI.transfer(h>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(w);
    SPI.transfer(h);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_FILLRECT);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(w>>8);
    SPI.transfer(w>>0);
    SPI.transfer(h>>8);
    SPI.transfer(h>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(w);
    SPI.transfer(h);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);

  return;
}


void DisplaySPI::fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
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


void DisplaySPI::drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWRNDRECT);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(w>>8);
    SPI.transfer(w>>0);
    SPI.transfer(h>>8);
    SPI.transfer(h>>0);
    SPI.transfer(r>>8);
    SPI.transfer(r>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(w);
    SPI.transfer(h);
    SPI.transfer(r);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_FILLRNDRECT);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(w>>8);
    SPI.transfer(w>>0);
    SPI.transfer(h>>8);
    SPI.transfer(h>>0);
    SPI.transfer(r>>8);
    SPI.transfer(r>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(w);
    SPI.transfer(h);
    SPI.transfer(r);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWCIRCLE);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(r>>8);
    SPI.transfer(r>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(r);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_FILLCIRCLE);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(r>>8);
    SPI.transfer(r>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(r);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWELLIPSE);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(r_x>>8);
    SPI.transfer(r_x>>0);
    SPI.transfer(r_y>>8);
    SPI.transfer(r_y>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(r_x);
    SPI.transfer(r_y);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


void DisplaySPI::fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  CS_ENABLE();
  SPI.transfer(CMD_LCD_FILLELLIPSE);
  if(bigger_8bit)
  {
    SPI.transfer(x0>>8);
    SPI.transfer(x0>>0);
    SPI.transfer(y0>>8);
    SPI.transfer(y0>>0);
    SPI.transfer(r_x>>8);
    SPI.transfer(r_x>>0);
    SPI.transfer(r_y>>8);
    SPI.transfer(r_y>>0);
  }
  else
  {
    SPI.transfer(x0);
    SPI.transfer(y0);
    SPI.transfer(r_x);
    SPI.transfer(r_y);
  }
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  CS_DISABLE();

  return;
}


int_least16_t DisplaySPI::drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  static uint_least32_t last_time=0;

  while(millis() == last_time); //slow down function calling
  last_time = millis();

  CS_ENABLE();
  SPI.transfer(CMD_LCD_DRAWTEXT);
  SPI.transfer(color>>8);
  SPI.transfer(color>>0);
  SPI.transfer(bg>>8);
  SPI.transfer(bg>>0);
  if(bigger_8bit)
  {
    SPI.transfer(x>>8);
    SPI.transfer(x>>0);
    SPI.transfer(y>>8);
    SPI.transfer(y>>0);
  }
  else
  {
    SPI.transfer(x);
    SPI.transfer(y);
  }
  SPI.transfer(size);
  SPI.transfer(1);
  SPI.transfer(c);
  CS_DISABLE();

  return x+(8*(size&0x7F));
}


int_least16_t DisplaySPI::drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  return drawChar(x, y, (char)c, color, bg, size);
}


int_least16_t DisplaySPI::drawText(int_least16_t x, int_least16_t y, char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;

  len = strlen(s);

  if((len != 0) && (len < 256))
  {
    CS_ENABLE();
    SPI.transfer(CMD_LCD_DRAWTEXT);
    SPI.transfer(color>>8);
    SPI.transfer(color>>0);
    SPI.transfer(bg>>8);
    SPI.transfer(bg>>0);
    if(bigger_8bit)
    {
      SPI.transfer(x>>8);
      SPI.transfer(x>>0);
      SPI.transfer(y>>8);
      SPI.transfer(y>>0);
    }
    else
    {
      SPI.transfer(x);
      SPI.transfer(y);
    }
    SPI.transfer(size);
    SPI.transfer(len);
    size = 8*(size&0x7F);
    while(*s != 0)
    {
      SPI.transfer((uint8_t)*s++);
      pos += size;
    }
    CS_DISABLE();
  }

  return pos;
}


int_least16_t DisplaySPI::drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  int_least16_t pos=x;

  if((s.length() != 0) && (s.length() < 256))
  {
    CS_ENABLE();
    SPI.transfer(CMD_LCD_DRAWTEXT);
    SPI.transfer(color>>8);
    SPI.transfer(color>>0);
    SPI.transfer(bg>>8);
    SPI.transfer(bg>>0);
    if(bigger_8bit)
    {
      SPI.transfer(x>>8);
      SPI.transfer(x>>0);
      SPI.transfer(y>>8);
      SPI.transfer(y>>0);
    }
    else
    {
      SPI.transfer(x);
      SPI.transfer(y);
    }
    SPI.transfer(size);
    SPI.transfer((uint8_t)s.length());
    size = 8*(size&0x7F);
    for(uint_least16_t i=0; i < s.length(); i++) 
    {
      SPI.transfer((uint8_t)s[i]);
      pos += size;
    }
    CS_DISABLE();
  }

  return pos;
}


#if defined(__AVR__)
int_least16_t DisplaySPI::drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  uint_least16_t len;
  int_least16_t pos=x;
  uint_least8_t c;

  len = strlen_P(s);

  if((len != 0) && (len < 256))
  {
    CS_ENABLE();
    SPI.transfer(CMD_LCD_DRAWTEXT);
    SPI.transfer(color>>8);
    SPI.transfer(color>>0);
    SPI.transfer(bg>>8);
    SPI.transfer(bg>>0);
    if(bigger_8bit)
    {
      SPI.transfer(x>>8);
      SPI.transfer(x>>0);
      SPI.transfer(y>>8);
      SPI.transfer(y>>0);
    }
    else
    {
      SPI.transfer(x);
      SPI.transfer(y);
    }
    SPI.transfer(size);
    SPI.transfer(len);
    size = 8*(size&0x7F);
    c = pgm_read_byte(s++);
    while(c != 0)
    {
      SPI.transfer(c);
      c = pgm_read_byte(s++);
      pos += size;
    }
    CS_DISABLE();
  }

  return pos;
}
#endif


uint_least8_t DisplaySPI::touchRead(void)
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

    CS_ENABLE();
    SPI.transfer(CMD_TP_POS);
    delay(1);

    if(bigger_8bit)
    {
      lcd_x  = SPI.transfer(0xFF)<<8;
      lcd_x |= SPI.transfer(0xFF)<<0;
      lcd_y  = SPI.transfer(0xFF)<<8;
      lcd_y |= SPI.transfer(0xFF)<<0;
      lcd_z  = SPI.transfer(0xFF)<<8;
      lcd_z |= SPI.transfer(0xFF)<<0;
    }
    else
    {
      lcd_x = SPI.transfer(0xFF);
      lcd_y = SPI.transfer(0xFF);
      lcd_z = SPI.transfer(0xFF);
    }

    CS_DISABLE();

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


void DisplaySPI::touchStartCal(void)
{
  uint_least32_t ms, last_time, timeout;

  if(!(features & FEATURE_TP))
  {
    return;
  }

  CS_ENABLE();
  SPI.transfer(CMD_TP_CALIBRATE);
  CS_DISABLE();

  for(last_time=0, timeout=0; 1;)
  { 
    ms = millis();
    if((ms-last_time) > 1000) //check every 1s
    {
      last_time = ms; //save time
      CS_ENABLE();
      int c = SPI.transfer(0xFF);
      CS_DISABLE();
      if(c == CMD_TP_CALIBRATE)
      {
        break;
      }
      if(++timeout >= 30) //timeout after 30s
      {
        CS_ENABLE();
        SPI.transfer(0x00);
        CS_DISABLE();
        break;
      }
    }
  }
  delay(200);
  CS_ENABLE();
  SPI.transfer(0xFF); //read response
  CS_DISABLE();
  delay(20);

  return;
}


int_least16_t DisplaySPI::touchX(void)
{
  return lcd_x;
}


int_least16_t DisplaySPI::touchY(void)
{
  return lcd_y;
}


int_least16_t DisplaySPI::touchZ(void)
{
  return lcd_z;
}
