#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "../SPI/SPI.h"
#include "../digitalWriteFast/digitalWriteFast.h"
#include "../GraphicsLib/GraphicsLib.h"
#include "MI0283QT9.h"


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


#define LCD_CMD_NOP                    0x00
#define LCD_CMD_RESET                  0x01
#define LCD_CMD_SLEEPIN                0x10
#define LCD_CMD_SLEEPOUT               0x11
#define LCD_CMD_PARTIAL_MODE           0x12
#define LCD_CMD_NORMAL_MODE            0x13
#define LCD_CMD_INV_OFF                0x20
#define LCD_CMD_INV_ON                 0x21
#define LCD_CMD_GAMMA                  0x26
#define LCD_CMD_DISPLAY_OFF            0x28
#define LCD_CMD_DISPLAY_ON             0x29
#define LCD_CMD_COLUMN                 0x2A
#define LCD_CMD_PAGE                   0x2B
#define LCD_CMD_WRITE                  0x2C
#define LCD_CMD_READ                   0x2E
#define LCD_CMD_PARTIAL_AREA           0x30
#define LCD_CMD_TEARING_OFF            0x34
#define LCD_CMD_TEARING_ON             0x35
#define LCD_CMD_MEMACCESS_CTRL         0x36
#define LCD_CMD_IDLE_OFF               0x38
#define LCD_CMD_IDLE_ON                0x39
#define LCD_CMD_PIXEL_FORMAT           0x3A
#define LCD_CMD_WRITE_CNT              0x3C
#define LCD_CMD_READ_CNT               0x3E
#define LCD_CMD_BRIGHTNESS             0x51
#define LCD_CMD_BRIGHTNESS_CTRL        0x53
#define LCD_CMD_RGB_CTRL               0xB0
#define LCD_CMD_FRAME_CTRL             0xB1 //normal mode
#define LCD_CMD_FRAME_CTRL_IDLE        0xB2 //idle mode
#define LCD_CMD_FRAME_CTRL_PART        0xB3 //partial mode
#define LCD_CMD_INV_CTRL               0xB4
#define LCD_CMD_DISPLAY_CTRL           0xB6
#define LCD_CMD_ENTRY_MODE             0xB7
#define LCD_CMD_POWER_CTRL1            0xC0
#define LCD_CMD_POWER_CTRL2            0xC1
#define LCD_CMD_VCOM_CTRL1             0xC5
#define LCD_CMD_VCOM_CTRL2             0xC7
#define LCD_CMD_POWER_CTRLA            0xCB
#define LCD_CMD_POWER_CTRLB            0xCF
#define LCD_CMD_POS_GAMMA              0xE0
#define LCD_CMD_NEG_GAMMA              0xE1
#define LCD_CMD_DRV_TIMING_CTRLA       0xE8
#define LCD_CMD_DRV_TIMING_CTRLB       0xEA
#define LCD_CMD_POWERON_SEQ_CTRL       0xED
#define LCD_CMD_ENABLE_3G              0xF2
#define LCD_CMD_INTERF_CTRL            0xF6
#define LCD_CMD_PUMP_RATIO_CTRL        0xF7


//-------------------- Constructor --------------------


MI0283QT9::MI0283QT9(void) : GraphicsLib(320, 240)
{
  return;
}


//-------------------- Public --------------------


void MI0283QT9::begin(uint_least8_t clock_div, uint_least8_t rst_pin)
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


void MI0283QT9::begin(uint_least8_t clock_div)
{
  return begin(clock_div, 0xFF);
}


void MI0283QT9::begin(void)
{
  return begin(SPI_CLOCK_DIV4, 0xFF);
}


void MI0283QT9::led(uint_least8_t power)
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


void MI0283QT9::invertDisplay(uint_least8_t invert)
{
  if(invert == 0)
  {
     wr_cmd(LCD_CMD_INV_OFF);
  }
  else
  {
     wr_cmd(LCD_CMD_INV_ON);
  }

  return;
}


void MI0283QT9::setOrientation(uint_least16_t o)
{
  #define MEM_Y   (7) //MY row address order
  #define MEM_X   (6) //MX column address order 
  #define MEM_V   (5) //MV row / column exchange 
  #define MEM_L   (4) //ML vertical refresh order
  #define MEM_H   (2) //MH horizontal refresh order
  #define MEM_BGR (3) //RGB-BGR Order 

  wr_cmd(LCD_CMD_MEMACCESS_CTRL);

  switch(o)
  {
    default:
    case 0:
      lcd_orientation = 0;
      lcd_width  = 320;
      lcd_height = 240;
      wr_data((1<<MEM_BGR) | (1<<MEM_X) | (1<<MEM_Y) | (1<<MEM_V));
      break;

    case  9:
    case 90:
      lcd_orientation = 90;
      lcd_width  = 240;
      lcd_height = 320;
      wr_data((1<<MEM_BGR) | (1<<MEM_X));
      break;

    case  18:
    case 180:
      lcd_orientation = 180;
      lcd_width  = 320;
      lcd_height = 240;
      wr_data((1<<MEM_BGR) | (1<<MEM_L) | (1<<MEM_V));
      break;

    case  27:
    case  14: //270&0xFF
    case 270:
      lcd_orientation = 270;
      lcd_width  = 240;
      lcd_height = 320;
      wr_data((1<<MEM_BGR) | (1<<MEM_Y));
      break;
  }
  
  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void MI0283QT9::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  if((x1 >= lcd_width) ||
     (y1 >= lcd_height))
  {
    return;
  }

  wr_cmd(LCD_CMD_COLUMN);
  wr_data16(x0);
  wr_data16(x1);

  wr_cmd(LCD_CMD_PAGE);
  wr_data16(y0);
  wr_data16(y1);

  return;
}


void MI0283QT9::drawStart(void)
{
  wr_cmd(LCD_CMD_WRITE);
  CS_ENABLE();

  return;
}


void MI0283QT9::draw(uint_least16_t color)
{
  //9th bit
  MOSI_HIGH(); //data
  CLK_LOW();
#if defined(SOFTWARE_SPI)
  CLK_HIGH();
#else if defined(__AVR__)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#endif

  wr_spi(color>>8);

  //9th bit
  MOSI_HIGH(); //data
  CLK_LOW();
#if defined(SOFTWARE_SPI)
  CLK_HIGH();
#else if defined(__AVR__)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#endif

  wr_spi(color);

  return;
}


void MI0283QT9::drawStop(void)
{
  CS_DISABLE();

  return;
}


//-------------------- Private --------------------


void MI0283QT9::reset(uint_least8_t clock_div, uint_least8_t rst_pin)
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
    delay(120);
  }

  //lcd_wrcmd(LCD_CMD_RESET);
  //delay(120);
  wr_cmd(LCD_CMD_DISPLAY_OFF);
  delay(20);

  //send init commands
  wr_cmd(LCD_CMD_POWER_CTRLB);
  wr_data(0x00);
  wr_data(0x83); //83 81 AA
  wr_data(0x30);

  wr_cmd(LCD_CMD_POWERON_SEQ_CTRL);
  wr_data(0x64); //64 67
  wr_data(0x03);
  wr_data(0x12);
  wr_data(0x81);

  wr_cmd(LCD_CMD_DRV_TIMING_CTRLA);
  wr_data(0x85);
  wr_data(0x01);
  wr_data(0x79); //79 78

  wr_cmd(LCD_CMD_POWER_CTRLA);
  wr_data(0x39);
  wr_data(0X2C);
  wr_data(0x00);
  wr_data(0x34);
  wr_data(0x02);

  wr_cmd(LCD_CMD_PUMP_RATIO_CTRL);
  wr_data(0x20);

  wr_cmd(LCD_CMD_DRV_TIMING_CTRLB);
  wr_data(0x00);
  wr_data(0x00);

  wr_cmd(LCD_CMD_POWER_CTRL1);
  wr_data(0x26); //26 25
  
  wr_cmd(LCD_CMD_POWER_CTRL2);
  wr_data(0x11);

  wr_cmd(LCD_CMD_VCOM_CTRL1);
  wr_data(0x35);
  wr_data(0x3E);

  wr_cmd(LCD_CMD_VCOM_CTRL2);
  wr_data(0xBE); //BE 94

  wr_cmd(LCD_CMD_FRAME_CTRL);
  wr_data(0x00);
  wr_data(0x1B); //1B 70

  //gamma control
  /*wr_cmd(LCD_CMD_ENABLE_3G);
  wr_data(0x08); //08 00
  wr_cmd(LCD_CMD_GAMMA);
  wr_data(0x01); //G2.2
  wr_cmd(LCD_CMD_POS_GAMMA);
  uint8_t pgama[15] = {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
  //uint8_t pgama[15] = {0x0F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
  for(uint8_t i=0; i<15; i++)
  {
    wr_data(pgama[i]);
  }
  wr_cmd(LCD_CMD_NEG_GAMMA);
  uint8_t ngama[15] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F};
  //uint8_t ngama[15] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x0F};
  for(uint8_t i=0; i<15; i++)
  {
    wr_data(ngama[i]);
  }*/

  wr_cmd(LCD_CMD_DISPLAY_CTRL);
  wr_data(0x0A);
  wr_data(0x82);
  wr_data(0x27);
  wr_data(0x00);

  wr_cmd(LCD_CMD_ENTRY_MODE);
  wr_data(0x07);

  wr_cmd(LCD_CMD_PIXEL_FORMAT);
  wr_data(0x55); //16bit

  //display options
  setOrientation(0);

  //clear display buffer
  fillScreen(0);

  //display on / sleep out
  wr_cmd(LCD_CMD_SLEEPOUT);
  delay(120);
  wr_cmd(LCD_CMD_DISPLAY_ON);
  delay(20);

  //restore SPI settings
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(clock_div);
#endif

  return;
}


void MI0283QT9::wr_cmd(uint_least8_t cmd)
{
  CS_ENABLE();

  //9th bit
  MOSI_LOW(); //cmd
  CLK_LOW();
#if defined(__AVR__) && !defined(SOFTWARE_SPI)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#else
  CLK_HIGH();
#endif

  wr_spi(cmd);

  CS_DISABLE();

  return;
}


void MI0283QT9::wr_data16(uint_least16_t data)
{
  CS_ENABLE();

  //9th bit
  MOSI_HIGH(); //data
  CLK_LOW();
#if defined(__AVR__) && !defined(SOFTWARE_SPI)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#else
  CLK_HIGH();
#endif

  wr_spi(data>>8);

  //9th bit
  MOSI_HIGH(); //data
  CLK_LOW();
#if defined(__AVR__) && !defined(SOFTWARE_SPI)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#else
  CLK_HIGH();
#endif

  wr_spi(data);

  CS_DISABLE();

  return;
}


void MI0283QT9::wr_data(uint_least8_t data)
{
  CS_ENABLE();

  //9th bit
  MOSI_HIGH(); //data
  CLK_LOW();
#if defined(__AVR__) && !defined(SOFTWARE_SPI)
  SPCR &= ~(1<<SPE); //disable SPI
  CLK_HIGH();
  SPCR |= (1<<SPE); //enable SPI
#else
  CLK_HIGH();
#endif

  wr_spi(data);

  CS_DISABLE();

  return;
}


void MI0283QT9::wr_spi(uint_least8_t data)
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
