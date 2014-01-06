#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
# include <util/delay.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "../SPI/SPI.h"
#include "../digitalWriteFast/digitalWriteFast.h"
#include "../GraphicsLib/GraphicsLib.h"
#include "MI0283QT2.h"


//#define SOFTWARE_SPI


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define LED_PIN        (9) //PH6
# define CS_PIN         (7)
# if defined(SOFTWARE_SPI)
#  define MOSI_PIN      (11)
#  define MISO_PIN      (12)
#  define CLK_PIN       (13)
# else
#  define MOSI_PIN      (51)
#  define MISO_PIN      (50)
#  define CLK_PIN       (52)
# endif

#elif (defined(__AVR_ATmega644__) || \
       defined(__AVR_ATmega644P__))    //--- Arduino 644 (www.mafu-foto.de) ---

# define LED_PIN        (3) //PB3
# define CS_PIN         (13)
# define MOSI_PIN       (5)
# define MISO_PIN       (6)
# define CLK_PIN        (7)

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define LED_PIN        (9) //PB5
# define CS_PIN         (7)
# define MOSI_PIN       (16) //PB2
# define MISO_PIN       (14) //PB3
# define CLK_PIN        (15) //PB1

#else                                  //--- Arduino Uno ---

# define LED_PIN        (9) //PB1
# define CS_PIN         (7)
# define MOSI_PIN       (11)
# define MISO_PIN       (12)
# define CLK_PIN        (13)

#endif


#define LED_ENABLE()    digitalWriteFast(LED_PIN, HIGH)
#define LED_DISABLE()   digitalWriteFast(LED_PIN, LOW)

#define CS_DISABLE()    digitalWriteFast(CS_PIN, HIGH)
#define CS_ENABLE()     digitalWriteFast(CS_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define MISO_READ()     digitalReadFast(MISO_PIN)

#define CLK_HIGH()      digitalWriteFast(CLK_PIN, HIGH)
#define CLK_LOW()       digitalWriteFast(CLK_PIN, LOW)


#define LCD_ID          (0)
#define LCD_DATA        ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER    ((0x70)|(LCD_ID<<2))


//-------------------- Constructor --------------------


MI0283QT2::MI0283QT2(void) : GraphicsLib(320, 240)
{
  return;
}


//-------------------- Public --------------------


void MI0283QT2::begin(uint_least8_t clock_div, uint_least8_t rst_pin)
{
  //init pins
  if(rst_pin < 0xFF)
  {
    pinMode(rst_pin, OUTPUT);
    digitalWrite(rst_pin, LOW);
  }
  pinMode(LED_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  LED_DISABLE();
  CS_DISABLE();

#if !defined(SOFTWARE_SPI)
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(clock_div);
  SPI.begin();
#endif

  //reset display
  reset(clock_div, rst_pin);

  //enable backlight
  led(50);

  return;
}


void MI0283QT2::begin(uint_least8_t clock_div)
{
  return begin(clock_div, 0xFF);
}


void MI0283QT2::begin(void)
{
  return begin(SPI_CLOCK_DIV4, 0xFF);
}


void MI0283QT2::led(uint_least8_t power)
{
  if(power == 0) //off
  {
    analogWrite(LED_PIN, 0);
    LED_DISABLE();
  }
  else if(power >= 100) //100%
  {
    analogWrite(LED_PIN, 255);
    LED_ENABLE();
  }
  else //1...99%
  {
    analogWrite(LED_PIN, (uint16_t)power*255/100);
  }

  return;
}


void MI0283QT2::setOrientation(uint_least16_t o)
{
  switch(o)
  {
    default:
    case 0:
      lcd_orientation = 0;
      lcd_width  = 320;
      lcd_height = 240;
      wr_cmd(0x16, 0xA8); //MY=1 MX=0 MV=1 ML=0 BGR=1
      break;

    case  9:
    case 90:
      lcd_orientation = 90;
      lcd_width  = 240;
      lcd_height = 320;
      wr_cmd(0x16, 0x08); //MY=0 MX=0 MV=0 ML=0 BGR=1
      break;

    case  18:
    case 180:
      lcd_orientation = 180;
      lcd_width  = 320;
      lcd_height = 240;
      wr_cmd(0x16, 0x68); //MY=0 MX=1 MV=1 ML=0 BGR=1
      break;

    case  27:
    case  14: //270&0xFF
    case 270:
      lcd_orientation = 270;
      lcd_width  = 240;
      lcd_height = 320;
      wr_cmd(0x16, 0xC8); //MY=1 MX=0 MV=1 ML=0 BGR=1
      break;
  }
  
  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void MI0283QT2::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  if((x1 >= lcd_width) ||
     (y1 >= lcd_height))
  {
    return;
  }

  wr_cmd(0x03, (x0>>0)); //set x0
  wr_cmd(0x02, (x0>>8)); //set x0
  wr_cmd(0x05, (x1>>0)); //set x1
  wr_cmd(0x04, (x1>>8)); //set x1
  wr_cmd(0x07, (y0>>0)); //set y0
  wr_cmd(0x06, (y0>>8)); //set y0
  wr_cmd(0x09, (y1>>0)); //set y1
  wr_cmd(0x08, (y1>>8)); //set y1

  return;
}


void MI0283QT2::fillScreen(uint_least16_t color)
{
  uint_least16_t size;

  setArea(0, 0, lcd_width-1, lcd_height-1);

  drawStart();
  for(size=(320UL*240UL/8UL); size!=0; size--)
  {
    draw(color); //1
    draw(color); //2
    draw(color); //3
    draw(color); //4
    draw(color); //5
    draw(color); //6
    draw(color); //7
    draw(color); //8
  }
  drawStop();

  return;
}


void MI0283QT2::drawStart(void)
{
  CS_ENABLE();
  wr_spi(LCD_REGISTER);
  wr_spi(0x22);
  CS_DISABLE();

  CS_ENABLE();
  wr_spi(LCD_DATA);

  return;
}


void MI0283QT2::draw(uint_least16_t color)
{
  wr_spi(color>>8);
  wr_spi(color);

  return;
}


void MI0283QT2::drawStop(void)
{
  CS_DISABLE();

  return;
}


//-------------------- Private --------------------


void MI0283QT2::reset(uint_least8_t clock_div, uint_least8_t rst_pin)
{
  //SPI speed-down
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(SPI_CLOCK_DIV8);
#endif

  //reset
  CS_DISABLE();
  if(rst_pin < 0xFF)
  {
    digitalWrite(rst_pin, LOW);
    delay(50);
    digitalWrite(rst_pin, HIGH);
    delay(50);
  }

  //driving ability
  wr_cmd(0xEA, 0x00);
  wr_cmd(0xEB, 0x20);
  wr_cmd(0xEC, 0x0C);
  wr_cmd(0xED, 0xC4);
  wr_cmd(0xE8, 0x40);
  wr_cmd(0xE9, 0x38);
  wr_cmd(0xF1, 0x01);
  wr_cmd(0xF2, 0x10);
  wr_cmd(0x27, 0xA3);

  //power voltage
  wr_cmd(0x1B, 0x1B);
  wr_cmd(0x1A, 0x01);
  wr_cmd(0x24, 0x2F);
  wr_cmd(0x25, 0x57);

  //VCOM offset
  wr_cmd(0x23, 0x8D); //for flicker adjust

  //power on
  wr_cmd(0x18, 0x36);
  wr_cmd(0x19, 0x01); //start osc
  wr_cmd(0x01, 0x00); //wakeup
  wr_cmd(0x1F, 0x88);
  _delay_ms(5);
  wr_cmd(0x1F, 0x80);
  _delay_ms(5);
  wr_cmd(0x1F, 0x90);
  _delay_ms(5);
  wr_cmd(0x1F, 0xD0);
  _delay_ms(5);

  //color selection
  wr_cmd(0x17, 0x05); //0x05=65k, 0x06=262k

  //panel characteristic
  wr_cmd(0x36, 0x00);

  //display options
  setOrientation(0);

  //clear display buffer
  fillScreen(0);

  //display on
  wr_cmd(0x28, 0x38);
  delay(50);
  wr_cmd(0x28, 0x3C);

  //restore SPI settings
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(clock_div);
#endif

  return;
}


void MI0283QT2::wr_cmd(uint_least8_t cmd, uint_least8_t param)
{
  CS_ENABLE();
  wr_spi(LCD_REGISTER);
  wr_spi(cmd);
  CS_DISABLE();

  CS_ENABLE();
  wr_spi(LCD_DATA);
  wr_spi(param);
  CS_DISABLE();

  return;
}


void MI0283QT2::wr_spi(uint_least8_t data)
{
#if defined(SOFTWARE_SPI)
  for(uint_least8_t mask=0x80; mask!=0; mask>>=1)
  {
    CLK_LOW();
    if(mask & data)
    {
      MOSI_HIGH();
    }
    else
    {
      MOSI_LOW();
    }
    CLK_HIGH();
  }
  CLK_LOW();
#else
  SPI.transfer(data);
#endif

  return;
}
