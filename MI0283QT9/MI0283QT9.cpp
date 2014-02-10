#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
# include <avr/pgmspace.h>
#else
# define pgm_read_byte(addr)  (*(const uint8_t *)(addr))
# define pgm_read_word(addr)  (*(const uint16_t *)(addr))
# define pgm_read_dword(addr) (*(const uint32_t *)(addr))
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


//#define SOFTWARE_SPI //use software SPI om pins 11,12,13

#define ADS7846 //enable ADS7846 support

#define MIN_PRESSURE    (5) //minimum pressure 1...254

#ifndef LCD_WIDTH
# define LCD_WIDTH      (320)
# define LCD_HEIGHT     (240)
#endif

#ifndef CAL_POINT_X1
# define CAL_POINT_X1   (20)
# define CAL_POINT_Y1   (20)
# define CAL_POINT1     {CAL_POINT_X1,CAL_POINT_Y1}
# define CAL_POINT_X2   LCD_WIDTH-20 //300
# define CAL_POINT_Y2   LCD_HEIGHT/2 //120
# define CAL_POINT2     {CAL_POINT_X2,CAL_POINT_Y2}
# define CAL_POINT_X3   LCD_WIDTH/2   //160
# define CAL_POINT_Y3   LCD_HEIGHT-20 //220
# define CAL_POINT3     {CAL_POINT_X3,CAL_POINT_Y3}
#endif


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define LED_PIN        (9) //PH6
# define RST_PIN        (8)
# define CS_PIN         (7)
# define ADSCS_PIN      (6)
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
# define RST_PIN        (12)
# define CS_PIN         (13)
# define ADSCS_PIN      (14)
# define MOSI_PIN       (5)
# define MISO_PIN       (6)
# define CLK_PIN        (7)

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define LED_PIN        (9) //PB5
# define RST_PIN        (8)
# define CS_PIN         (7)
# define ADSCS_PIN      (6)
# if defined(SOFTWARE_SPI)
#  define MOSI_PIN      (11)
#  define MISO_PIN      (12)
#  define CLK_PIN       (13)
# else
#  define MOSI_PIN      (16) //PB2
#  define MISO_PIN      (14) //PB3
#  define CLK_PIN       (15) //PB1
# endif

#else                                  //--- Arduino Uno ---

# define LED_PIN        (9) //PB1
# define RST_PIN        (8)
# define CS_PIN         (7)
# define ADSCS_PIN      (6)
# define MOSI_PIN       (11)
# define MISO_PIN       (12)
# define CLK_PIN        (13)

#endif


#define LED_ENABLE()    digitalWriteFast(LED_PIN, HIGH)
#define LED_DISABLE()   digitalWriteFast(LED_PIN, LOW)

#define RST_DISABLE()   digitalWriteFast(RST_PIN, HIGH)
#define RST_ENABLE()    digitalWriteFast(RST_PIN, LOW)

#define CS_DISABLE()    digitalWriteFast(CS_PIN, HIGH)
#define CS_ENABLE()     digitalWriteFast(CS_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define MISO_READ()     digitalReadFast(MISO_PIN)

#define CLK_HIGH()      digitalWriteFast(CLK_PIN, HIGH)
#define CLK_LOW()       digitalWriteFast(CLK_PIN, LOW)

#define ADSCS_DISABLE() digitalWriteFast(ADSCS_PIN, HIGH)
#define ADSCS_ENABLE()  digitalWriteFast(ADSCS_PIN, LOW)


//LCD commands
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

//ADS commands
#define ADS_CMD_START                  0x80
#define ADS_CMD_12BIT                  0x00
#define ADS_CMD_8BIT                   0x08
#define ADS_CMD_DIFF                   0x00
#define ADS_CMD_SINGLE                 0x04
#define ADS_CMD_X_POS                  0x10
#define ADS_CMD_Z1_POS                 0x30
#define ADS_CMD_Z2_POS                 0x40
#define ADS_CMD_Y_POS                  0x50
#define ADS_CMD_PWD                    0x00
#define ADS_CMD_ALWAYSON               0x03


//-------------------- Constructor --------------------


MI0283QT9::MI0283QT9(void) : GraphicsLib(LCD_WIDTH, LCD_HEIGHT)
{
  return;
}


//-------------------- Public --------------------


void MI0283QT9::begin(uint_least8_t clock_div)
{
  //init pins
#if defined(RST_PIN)
  pinMode(RST_PIN, OUTPUT);
  RST_ENABLE();
#endif
  pinMode(LED_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  LED_DISABLE();
  CS_DISABLE();
#if defined(ADS7846)
  pinMode(ADSCS_PIN, OUTPUT);
  ADSCS_DISABLE();
#endif

#if !defined(SOFTWARE_SPI)
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(clock_div);
  SPI.begin();
#endif

  //reset display
  reset(clock_div);

  //enable backlight
  led(50);

  return;
}


void MI0283QT9::begin(void)
{
  return begin(SPI_CLOCK_DIV4);
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
  uint_least8_t p;

  #define MEM_Y   (7) //MY row address order
  #define MEM_X   (6) //MX column address order 
  #define MEM_V   (5) //MV row / column exchange 
  #define MEM_L   (4) //ML vertical refresh order
  #define MEM_H   (2) //MH horizontal refresh order
  #define MEM_BGR (3) //RGB-BGR Order 

       if((o ==   9) || 
          (o ==  90))
  {
    lcd_orientation = 90;
    lcd_width  = LCD_HEIGHT;
    lcd_height = LCD_WIDTH;
    p = (1<<MEM_BGR) | (1<<MEM_X);
  }

  else if((o ==  18) || 
          (o == 180))
  {
    lcd_orientation = 180;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = (1<<MEM_BGR) | (1<<MEM_L) | (1<<MEM_V);
  }

  else if((o ==  27) || 
          (o ==  14) || 
          (o == 270))
  {
    lcd_orientation = 270;
    lcd_width  = LCD_HEIGHT;
    lcd_height = LCD_WIDTH;
    p = (1<<MEM_BGR) | (1<<MEM_Y);
  }

  else
  {
    lcd_orientation = 0;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = (1<<MEM_BGR) | (1<<MEM_X) | (1<<MEM_Y) | (1<<MEM_V);
  }

  wr_cmd(LCD_CMD_MEMACCESS_CTRL);
  wr_data(p);

  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void MI0283QT9::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
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


uint_least8_t MI0283QT9::touchRead(void)
{
#if defined(ADS7846)
  uint_least8_t p, a1, a2, b1, b2;
  uint_least16_t x, y;

  //SPI speed-down
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# if F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# else //elif F_CPU >= 4000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV2);
# endif
#endif

  //get pressure
  ADSCS_ENABLE();
  wr_spi(ADS_CMD_START | ADS_CMD_8BIT | ADS_CMD_DIFF | ADS_CMD_Z1_POS);
  a1 = rd_spi();
  wr_spi(ADS_CMD_START | ADS_CMD_8BIT | ADS_CMD_DIFF | ADS_CMD_Z2_POS);
  b1 = 127-(rd_spi()&0x7F);
  ADSCS_DISABLE();
  p = a1 + b1;

  if(p > MIN_PRESSURE)
  {
    /*//using 4 samples for x and y position
    for(x=0, y=0, i=4; i!=0; i--)
    {
      ADSCS_ENABLE();
      //get X data
      wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_X_POS);
      a = rd_spi();
      b = rd_spi();
      x += 1023-((a<<2)|(b>>6)); //12bit: ((a<<4)|(b>>4)) //10bit: ((a<<2)|(b>>6))
      //get Y data
      wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_Y_POS);
      a = rd_spi();
      b = rd_spi();
      y += ((a<<2)|(b>>6)); //12bit: ((a<<4)|(b>>4)) //10bit: ((a<<2)|(b>>6))
      ADSCS_DISABLE();
    }
    x >>= 2; //x/4
    y >>= 2; //y/4*/
    
    //using 2 samples for x and y position
    ADSCS_ENABLE();
    //get X data
    wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_X_POS);
    a1 = rd_spi();
    b1 = rd_spi();
    wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_X_POS);
    a2 = rd_spi();
    b2 = rd_spi();

    if(a1 == a2)
    {
      x = 1023-((a2<<2)|(b2>>6)); //12bit: ((a<<4)|(b>>4)) //10bit: ((a<<2)|(b>>6))

      //get Y data
      wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_Y_POS);
      a1 = rd_spi();
      b1 = rd_spi();
      wr_spi(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DIFF | ADS_CMD_Y_POS);
      a2 = rd_spi();
      b2 = rd_spi();

      if(a1 == a2)
      {
        y = ((a2<<2)|(b2>>6)); //12bit: ((a<<4)|(b>>4)) //10bit: ((a<<2)|(b>>6))
        if(x && y)
        {
          tp_x = x;
          tp_y = y;
        }
        lcd_z = p;
      }
    }
    ADSCS_DISABLE();
  }
  else
  {
    lcd_z = 0;
  }

  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif

  if(lcd_z != 0)
  {
    return 1;
  }
#endif
  return 0;
}


void MI0283QT9::touchStartCal(void)
{
  uint_least8_t i;
  CAL_POINT lcd_points[3] = {CAL_POINT1, CAL_POINT2, CAL_POINT3}; //calibration point postions
  CAL_POINT tp_points[3];
  uint_least16_t o;

  //clear screen
  o = getOrientation();
  setOrientation(0);
  fillScreen(RGB(255,255,255));
#if defined(__AVR__)
  drawTextPGM((lcd_width/2)-50, (lcd_height/2)-10, PSTR("Calibration"), RGB(0,0,0), RGB(255,255,255), 1);
#else
  drawText((lcd_width/2)-50, (lcd_height/2)-10, "Calibration", RGB(0,0,0), RGB(255,255,255), 1);
#endif
  while(touchRead()); //wait for touch release

  //show calibration points
  for(i=0; i<3; )
  {
    //draw points
    drawCircle(lcd_points[i].x, lcd_points[i].y,  2, RGB(  0,  0,  0));
    drawCircle(lcd_points[i].x, lcd_points[i].y,  5, RGB(  0,  0,  0));
    drawCircle(lcd_points[i].x, lcd_points[i].y, 10, RGB(255,  0,  0));

    //press dectected? -> save point
    if(touchRead())
    {
      fillCircle(lcd_points[i].x, lcd_points[i].y, 2, RGB(255,0,0));
      tp_points[i].x = tp_x;
      tp_points[i].y = tp_y;
      i++;

      //wait and redraw screen
      delay(100);
      fillScreen(RGB(255,255,255));
      if(i < 3)
      {
#if defined(__AVR__)
        drawTextPGM((lcd_width/2)-50, (lcd_height/2)-10, PSTR("Calibration"), RGB(0,0,0), RGB(255,255,255), 1);
#else
        drawText((lcd_width/2)-50, (lcd_height/2)-10, "Calibration", RGB(0,0,0), RGB(255,255,255), 1);
#endif
      }
    }
  }

  //calulate calibration matrix
  touchSetCal(lcd_points, tp_points);
  
  //restore orientation
  setOrientation(o);

  //wait for touch release
  while(touchRead()); //wait for touch release

  return;
}


//-------------------- Private --------------------


#if defined(__AVR__)
//const prog_uint8_t initdataQT9[] = 
const uint8_t initdataQT9[] PROGMEM = 
#else
const uint8_t initdataQT9[] = 
#endif
{
  //0x40| 1, LCD_CMD_RESET,
  //0xC0|60, //60ms
  //0xC0|60, //60ms
  0x40| 1, LCD_CMD_DISPLAY_OFF,
  0xC0|20, //20ms
  0x40| 1, LCD_CMD_POWER_CTRLB,
  0x80| 3, 0x00, 0x83, 0x30, //0x83 0x81 0xAA
  0x40| 1, LCD_CMD_POWERON_SEQ_CTRL,
  0x80| 4, 0x64, 0x03, 0x12, 0x81, //0x64 0x67
  0x40| 1, LCD_CMD_DRV_TIMING_CTRLA,
  0x80| 3, 0x85, 0x01, 0x79, //0x79 0x78
  0x40| 1, LCD_CMD_POWER_CTRLA,
  0x80| 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0x40| 1, LCD_CMD_PUMP_RATIO_CTRL,
  0x80| 1, 0x20,
  0x40| 1, LCD_CMD_DRV_TIMING_CTRLB,
  0x80| 2, 0x00, 0x00,
  0x40| 1, LCD_CMD_POWER_CTRL1,
  0x80| 1, 0x26, //0x26 0x25
  0x40| 1, LCD_CMD_POWER_CTRL2,
  0x80| 1, 0x11,
  0x40| 1, LCD_CMD_VCOM_CTRL1,
  0x80| 2, 0x35, 0x3E,
  0x40| 1, LCD_CMD_VCOM_CTRL2,
  0x80| 1, 0xBE, //0xBE 0x94
  0x40| 1, LCD_CMD_FRAME_CTRL,
  0x80| 2, 0x00, 0x1B, //0x1B 0x70
  0x40| 1, LCD_CMD_ENABLE_3G,
  0x80| 1, 0x08, //0x08 0x00
  0x40| 1, LCD_CMD_GAMMA,
  0x80| 1, 0x01, //G2.2
  0x40| 1, LCD_CMD_POS_GAMMA,
  0x80|15, 0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
//0x80|15, 0x0F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
  0x40| 1, LCD_CMD_NEG_GAMMA,
  0x80|15, 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F,
//0x80|15, 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x0F,
  0x40| 1, LCD_CMD_DISPLAY_CTRL,
  0x80| 4, 0x0A, 0x82, 0x27, 0x00,
  0x40| 1, LCD_CMD_ENTRY_MODE,
  0x80| 1, 0x07,
  0x40| 1, LCD_CMD_PIXEL_FORMAT,
  0x80| 1, 0x55, //16bit
  0x40| 1, LCD_CMD_MEMACCESS_CTRL,
  0x80| 1, (1<<MEM_BGR) | (1<<MEM_X) | (1<<MEM_Y) | (1<<MEM_V),
  0x40| 1, LCD_CMD_COLUMN,
  0x80| 2, 0x00, 0x00,
  0x80| 2, ((LCD_WIDTH-1)>>8)&0xFF, (LCD_WIDTH-1)&0xFF,
  0x40| 1, LCD_CMD_PAGE,
  0x80| 2, 0x00, 0x00,
  0x80| 2, ((LCD_HEIGHT-1)>>8)&0xFF, (LCD_HEIGHT-1)&0xFF,
  0x40| 1, LCD_CMD_SLEEPOUT,
  0xC0|60, //60ms
  0xC0|60, //60ms
  0x40| 1, LCD_CMD_DISPLAY_ON,
  0xC0|20, //20ms
  0xFF   , 0xFF
};


void MI0283QT9::reset(uint_least8_t clock_div)
{
  uint_least8_t c, i;
#if defined(__AVR__)
  const prog_uint8_t *ptr;
#else
  uint8_t *ptr;
#endif

  //SPI speed-down
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(SPI_CLOCK_DIV8);
#endif

  //reset
  CS_DISABLE();
#if defined(RST_PIN)
  RST_ENABLE();
  delay(50);
  RST_DISABLE();
  delay(120);
#endif

  //send init commands and data
  ptr = &initdataQT9[0];
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
          wr_data(c);
        }
        break;
      case 0xC0: //delay
        delay(c&0x3F);
        break;
    }
  }

  //clear display buffer
  fillScreen(0);

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


uint_least8_t MI0283QT9::rd_spi(void)
{
#if defined(SOFTWARE_SPI)
  uint_least8_t data=0;
  MOSI_LOW();
  for(uint_least8_t bit=8; bit!=0; bit--)
  {
    CLK_HIGH();
    data <<= 1;
    if(MISO_READ())
    {
      data |= 1;
    }
    else
    {
      //data |= 0;
    }
    CLK_LOW();
  }
  return data;
#else
  return SPI.transfer(0x00);
#endif
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
