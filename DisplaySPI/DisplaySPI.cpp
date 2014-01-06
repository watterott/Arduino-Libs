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
#include "../SPI/SPI.h"
#include "../Wire/Wire.h"
#include "../digitalWriteFast/digitalWriteFast.h"
#include "../GraphicsLib/GraphicsLib.h"
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

  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(clock_div);
  SPI.begin();

  //reset display
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

  setOrientation(0); //display rotation
  fillScreen(0); //clear display buffer
  led(50); //enable backlight

  return;
}


void DisplaySPI::begin(uint_least8_t clock_div)
{
  return begin(clock_div, 0xFF);
}


void DisplaySPI::begin(void)
{
  return begin(SPI_CLOCK_DIV4, 0xFF);
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

  CS_ENABLE();
  SPI.transfer(CMD_LCD_ORIENTATION);
  SPI.transfer(lcd_orientation/10); //0, 9, 18, 27
  CS_DISABLE();

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
