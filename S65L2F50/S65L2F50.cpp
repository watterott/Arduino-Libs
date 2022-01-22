/*
  S65 L2F50 Display Lib for Arduino
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
#include "SPI.h"
#include "digitalWriteFast.h"
#include "GraphicsLib.h"
#include "S65L2F50.h"


//#define SOFTWARE_SPI //use software SPI on pins MOSI:11, MISO:12, SCK:13

#if defined(SOFTWARE_SPI)
# define RST_PIN        17 //A3=17
# define CS_PIN         16 //A2=16
# define RS_PIN         4
# define MOSI_PIN       SPI_SW_MOSI_PIN
# define MISO_PIN       SPI_SW_MISO_PIN
# define SCK_PIN        SPI_SW_SCK_PIN
#else
# define RST_PIN        17 //A3=17
# define CS_PIN         16 //A2=16
# define RS_PIN         4
# define MOSI_PIN       SPI_HW_MOSI_PIN
# define MISO_PIN       SPI_HW_MISO_PIN
# define SCK_PIN        SPI_HW_SCK_PIN
#endif

#define RST_DISABLE()   digitalWriteFast(RST_PIN, HIGH)
#define RST_ENABLE()    digitalWriteFast(RST_PIN, LOW)

#define CS_DISABLE()    digitalWriteFast(CS_PIN, HIGH)
#define CS_ENABLE()     digitalWriteFast(CS_PIN, LOW)

#define RS_DISABLE()    digitalWriteFast(RS_PIN, HIGH)
#define RS_ENABLE()     digitalWriteFast(RS_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define SCK_HIGH()      digitalWriteFast(SCK_PIN, HIGH)
#define SCK_LOW()       digitalWriteFast(SCK_PIN, LOW)


#ifndef LCD_WIDTH
# define LCD_WIDTH      (176)
# define LCD_HEIGHT     (132)
#endif


//-------------------- Constructor --------------------


S65L2F50::S65L2F50(void) : GraphicsLib(LCD_WIDTH, LCD_HEIGHT)
{
  return;
}


//-------------------- Public --------------------


void S65L2F50::begin(uint_least8_t clock_div)
{
  //init pins
#if defined(RST_PIN)
  pinMode(RST_PIN, OUTPUT);
  RST_ENABLE();
#endif
  pinMode(CS_PIN, OUTPUT);
  pinMode(RS_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  CS_DISABLE();

#if !defined(SOFTWARE_SPI)
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  //SPI.setClockDivider(clock_div);
  SPI.begin();
#endif

  //SPI speed-down
#if !defined(SOFTWARE_SPI)
# if F_CPU >= 128000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV64);
# elif F_CPU >= 64000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV32);
# elif F_CPU >= 32000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV16);
# elif F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# else //elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# endif
#endif

  //reset display
  reset();

  //SPI speed-up
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(clock_div);
#endif

  return;
}


void S65L2F50::begin(void)
{
  return begin(SPI_CLOCK_DIV4);
}


void S65L2F50::led(uint_least8_t power)
{
  return;
}


void S65L2F50::setOrientation(uint_least16_t o)
{
  uint_least8_t p;

       if((o ==   9) || 
          (o ==  90))
  {
    //not implemented
    p = 0;
  }

  else if((o ==  18) || 
          (o == 180))
  {
    lcd_orientation = 180;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = 0x2A; //565 mode, 0x2A=normal, 0x2B=180
  }

  else if((o ==  27) || 
          (o ==  14) || 
          (o == 270))
  {
    //not implemented
    p = 0;
  }

  else
  {
    lcd_orientation = 0;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = 0x2B; //565 mode, 0x2A=normal, 0x2B=180
  }

  if(p != 0)
  {
    wr_cmd(0xBC); //data control
    wr_dat(p); //565 mode, 0x2A=normal, 0x2B=180
  }

  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void S65L2F50::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  switch(lcd_orientation)
  {
    default:
    case 0:
      wr_cmd(0x15);                   //column address set 
      wr_dat(0x08+y0);                //start column
      wr_dat(0x01);                   //start column
      wr_dat(0x08+y1);                //end column
      wr_dat(0x01);                   //end column
      wr_cmd(0x75);                   //page address set 
      wr_dat(x0);                     //start page
      wr_dat(x1);                     //end page
      break;
    //case 90:
      //not implemented
    //  break;
    case 180:
      wr_cmd(0x15);                   //column address set 
      wr_dat(0x08+(LCD_HEIGHT-1)-y1); //start column
      wr_dat(0x01);                   //start column
      wr_dat(0x08+(LCD_HEIGHT-1)-y0); //end column
      wr_dat(0x01);                   //end column
      wr_cmd(0x75);                   //page address set 
      wr_dat(x0);                     //start page
      wr_dat(x1);                     //end page
      break;
    //case 270:
      //not implemented
    //  break;
  }

  return;
}


void S65L2F50::drawStart(void)
{
  wr_cmd(0x5C);
  RS_DISABLE(); //data
  CS_ENABLE();

  return;
}


void S65L2F50::draw(uint_least16_t color)
{
  wr_spi(color>>8);
  wr_spi(color);

  return;
}


void S65L2F50::drawStop(void)
{
  CS_DISABLE();

  return;
}


//-------------------- Private --------------------


const uint8_t initdataL2F50[] PROGMEM = 
{
  //display off
  0x40| 1, 0xAE,
  //display control 
  0x40| 1, 0xCA,
  0x80| 1, 0x4C, //P1
  0x80| 1, 0x01, //P2
  0x80| 1, 0x53, //P3
  0x80| 1, 0x00, //P4
  0x80| 1, 0x02, //P5
  0x80| 1, 0xB4, //P6
  0x80| 1, 0xB0, //P7
  0x80| 1, 0x02, //P8
  0x80| 1, 0x00, //P9
  //sleep out
  0x40| 1, 0x94,
  0xC0| 5, //5ms
  //display options
  0x40| 1, 0xBC, //data control
  0x80| 1, 0x2B, //565 mode, 0x2A=normal, 0x2B=180
  0x40| 1, 0x15,              //column address set 
  0x80| 1, 0x08+0,            //start column
  0x80| 1, 0x01,              //start column
  0x80| 1, 0x08+LCD_HEIGHT-1, //end column
  0x80| 1, 0x01,              //end column
  0x40| 1, 0x75,              //page address set 
  0x80| 1, 0,                 //start page
  0x80| 1, LCD_WIDTH-1,       //end page
  //display on
  0x40| 1, 0xAF,
  0xC0|10, //10ms
  0xFF   , 0xFF
};


void S65L2F50::reset(void)
{
  uint_least8_t c, i;
  const PROGMEM uint8_t *ptr;

  //reset
  CS_DISABLE();
  RS_DISABLE();
#if defined(RST_PIN)
  RST_ENABLE();
  delay(50);
  RST_DISABLE();
  delay(120);
#endif

  //send init commands and data
  ptr = &initdataL2F50[0];
  while(1)
  {
    c = pgm_read_byte(ptr++);
    if(c == 0xFF) //end of data
    {
      break;
    }
    switch(c&0xC0)
    {
      case 0x40: //command
        for(i=c&0x3F; i!=0; i--)
        {
          c = pgm_read_byte(ptr++);
          wr_cmd(c);
        }
        break;
      case 0x80: //data
        for(i=c&0x3F; i!=0; i--)
        {
          c = pgm_read_byte(ptr++);
          wr_dat(c);
        }
        break;
      case 0xC0: //delay
        c = c&0x3F;
        delay(c);
        break;
    }
  }

  //clear display buffer
  fillScreen(0);

  return;
}


void S65L2F50::wr_cmd(uint_least8_t cmd)
{
  RS_ENABLE(); //cmd
  CS_ENABLE();
  wr_spi(cmd);
  wr_spi(0x00);
  CS_DISABLE();

  return;
}


void S65L2F50::wr_dat(uint_least8_t data)
{
  RS_DISABLE(); //data
  CS_ENABLE();
  wr_spi(data);
  wr_spi(0x00);
  CS_DISABLE();

  return;
}


void S65L2F50::wr_spi(uint_least8_t data)
{
#if defined(SOFTWARE_SPI)
  for(uint_least8_t mask=0x80; mask!=0; mask>>=1)
  {
    SCK_LOW();
    if(mask & data)
    {
      MOSI_HIGH();
    }
    else
    {
      MOSI_LOW();
    }
    SCK_HIGH();
  }
  SCK_LOW();
#else
  SPI.transfer(data);
#endif
  return;
}
