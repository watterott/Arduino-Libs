/*
  MI0283QT-2 (HX8347D) Display Lib for Arduino
  by Watterott electronic (www.watterott.com)
 */

#include <inttypes.h>
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
# include <avr/io.h>
# include <avr/pgmspace.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "SPI.h"
#include "digitalWriteFast.h"
#include "GraphicsLib.h"
#include "MI0283QT2.h"


#define ADS7846 //enable ADS7846 support

#define MIN_PRESSURE    5 //minimum pressure 1...254

//#define SOFTWARE_SPI //use software SPI on pins MOSI:11, MISO:12, SCK:13

#if defined(SOFTWARE_SPI)
# define LED_PIN        9 //PB1
# define RST_PIN        8
# define CS_PIN         7 //SPI_SW_SS_PIN
# define ADSCS_PIN      6
# define MOSI_PIN       SPI_SW_MOSI_PIN
# define MISO_PIN       SPI_SW_MISO_PIN
# define SCK_PIN        SPI_SW_SCK_PIN
#else
# define LED_PIN        9 //PB1
# define RST_PIN        8
# define CS_PIN         7 //SPI_HW_SS_PIN
# define ADSCS_PIN      6
# define MOSI_PIN       SPI_HW_MOSI_PIN
# define MISO_PIN       SPI_HW_MISO_PIN
# define SCK_PIN        SPI_HW_SCK_PIN
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

#define SCK_HIGH()      digitalWriteFast(SCK_PIN, HIGH)
#define SCK_LOW()       digitalWriteFast(SCK_PIN, LOW)

#define ADSCS_DISABLE() digitalWriteFast(ADSCS_PIN, HIGH)
#define ADSCS_ENABLE()  digitalWriteFast(ADSCS_PIN, LOW)


#ifndef LCD_WIDTH
# define LCD_WIDTH      320
# define LCD_HEIGHT     240
#endif

#ifndef CAL_POINT_X1
# define CAL_POINT_X1   20
# define CAL_POINT_Y1   20
# define CAL_POINT1     {CAL_POINT_X1,CAL_POINT_Y1}
# define CAL_POINT_X2   LCD_WIDTH-20 //300
# define CAL_POINT_Y2   LCD_HEIGHT/2 //120
# define CAL_POINT2     {CAL_POINT_X2,CAL_POINT_Y2}
# define CAL_POINT_X3   LCD_WIDTH/2   //160
# define CAL_POINT_Y3   LCD_HEIGHT-20 //220
# define CAL_POINT3     {CAL_POINT_X3,CAL_POINT_Y3}
#endif

//LCD commands
#define LCD_ID          0
#define LCD_DATA        ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER    ((0x70)|(LCD_ID<<2))

//ADS commands
#define ADS_CMD_START        0x80
#define ADS_CMD_12BIT        0x00
#define ADS_CMD_8BIT         0x08
#define ADS_CMD_DIFF         0x00
#define ADS_CMD_SINGLE       0x04
#define ADS_CMD_X_POS        0x10
#define ADS_CMD_Z1_POS       0x30
#define ADS_CMD_Z2_POS       0x40
#define ADS_CMD_Y_POS        0x50
#define ADS_CMD_PWD          0x00
#define ADS_CMD_ALWAYSON     0x03


//-------------------- Constructor --------------------


MI0283QT2::MI0283QT2(void) : GraphicsLib(LCD_WIDTH, LCD_HEIGHT)
{
  return;
}


//-------------------- Public --------------------


void MI0283QT2::begin(uint_least8_t clock_div)
{
  //init pins
#if defined(RST_PIN)
  pinMode(RST_PIN, OUTPUT);
  RST_ENABLE();
#endif
  pinMode(LED_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
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


void MI0283QT2::begin(void)
{
  return begin(SPI_CLOCK_DIV4);
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
  uint_least8_t p;

       if((o ==   9) || 
          (o ==  90))
  {
    lcd_orientation = 90;
    lcd_width  = LCD_HEIGHT;
    lcd_height = LCD_WIDTH;
    p = 0x08; //MY=0 MX=0 MV=0 ML=0 BGR=1
  }

  else if((o ==  18) || 
          (o == 180))
  {
    lcd_orientation = 180;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = 0x68; //MY=0 MX=1 MV=1 ML=0 BGR=1
  }

  else if((o ==  27) || 
          (o ==  14) || 
          (o == 270))
  {
    lcd_orientation = 270;
    lcd_width  = LCD_HEIGHT;
    lcd_height = LCD_WIDTH;
    p = 0xC8; //MY=1 MX=0 MV=1 ML=0 BGR=1
  }

  else
  {
    lcd_orientation = 0;
    lcd_width  = LCD_WIDTH;
    lcd_height = LCD_HEIGHT;
    p = 0xA8; //MY=1 MX=0 MV=1 ML=0 BGR=1
  }

  wr_cmd(0x16, p);

  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void MI0283QT2::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
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


uint_least8_t MI0283QT2::touchRead(void)
{
#if defined(ADS7846)
  uint_least8_t p, a1, a2, b1, b2;
  uint_least16_t x, y;

  //SPI speed-down
#if !defined(SOFTWARE_SPI) && (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
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
  a1 = rd_spi()&0x7F;
  wr_spi(ADS_CMD_START | ADS_CMD_8BIT | ADS_CMD_DIFF | ADS_CMD_Z2_POS);
  b1 = (255-rd_spi())&0x7F;
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
#if !defined(SOFTWARE_SPI) && (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
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


void MI0283QT2::touchStartCal(void)
{
  uint_least8_t i;
  CAL_POINT lcd_points[3] = {CAL_POINT1, CAL_POINT2, CAL_POINT3}; //calibration point postions
  CAL_POINT tp_points[3];
  uint_least16_t o;

  //clear screen
  o = getOrientation();
  setOrientation(0);
  fillScreen(RGB(255,255,255));
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
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
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
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


const uint8_t initdataQT2[] PROGMEM = 
{
  //driving ability
  0x40| 2, 0xEA, 0x00,
  0x40| 2, 0xEB, 0x20,
  0x40| 2, 0xEC, 0x0C,
  0x40| 2, 0xED, 0xC4,
  0x40| 2, 0xE8, 0x40,
  0x40| 2, 0xE9, 0x38,
  0x40| 2, 0xF1, 0x01,
  0x40| 2, 0xF2, 0x10,
  0x40| 2, 0x27, 0xA3,
  //power voltage
  0x40| 2, 0x1B, 0x1B,
  0x40| 2, 0x1A, 0x01,
  0x40| 2, 0x24, 0x2F,
  0x40| 2, 0x25, 0x57,
  //VCOM offset
  0x40| 2, 0x23, 0x8D,
  //power on
  0x40| 2, 0x18, 0x36,
  0x40| 2, 0x19, 0x01, //start osc
  0x40| 2, 0x01, 0x00, //wakeup
  0x40| 2, 0x1F, 0x88,
  0xC0| 5, //5ms
  0x40| 2, 0x1F, 0x80,
  0xC0| 5, //5ms
  0x40| 2, 0x1F, 0x90,
  0xC0| 5, //5ms
  0x40| 2, 0x1F, 0xD0,
  0xC0| 5, //5ms
  //color selection
  0x40| 2, 0x17, 0x05, //0x05=65k, 0x06=262k
  //panel characteristic
  0x40| 2, 0x36, 0x00,
  //display options
  0x40| 2, 0x16, 0xA8,
  0x40| 2, 0x03, 0x00, //x0
  0x40| 2, 0x02, 0x00, //x0
  0x40| 2, 0x05, ((LCD_WIDTH-1)>>0)&0xFF,
  0x40| 2, 0x04, ((LCD_WIDTH-1)>>8)&0xFF,
  0x40| 2, 0x07, 0x00, //y0
  0x40| 2, 0x06, 0x00, //y0
  0x40| 2, 0x09, ((LCD_HEIGHT-1)>>0)&0xFF,
  0x40| 2, 0x08, ((LCD_HEIGHT-1)>>8)&0xFF,
  //display on
  0x40| 2, 0x28, 0x38,
  0xC0|50, //50ms
  0x40| 2, 0x28, 0x3C,
  0xC0| 5, //5ms
  0xFF   , 0xFF
};


void MI0283QT2::reset(uint_least8_t clock_div)
{
  uint_least8_t c, d, i;
  const PROGMEM uint8_t *ptr;

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
  ptr = &initdataQT2[0];
  while(1)
  {
    c = pgm_read_byte(ptr++);
    if(c == 0xFF) //end of data
    {
      break;
    }
    switch(c&0xC0)
    {
      case 0x40: //command + data
        for(i=c&0x3F; i!=0; i-=2)
        {
          c = pgm_read_byte(ptr++);
          d = pgm_read_byte(ptr++);
          wr_cmd(c, d);
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


uint_least8_t MI0283QT2::rd_spi(void)
{
#if defined(SOFTWARE_SPI)
  uint_least8_t data=0;
  MOSI_LOW();
  for(uint_least8_t bit=8; bit!=0; bit--)
  {
    SCK_HIGH();
    data <<= 1;
    if(MISO_READ())
    {
      data |= 1;
    }
    else
    {
      //data |= 0;
    }
    SCK_LOW();
  }
  return data;
#else
  return SPI.transfer(0x00);
#endif
}


void MI0283QT2::wr_spi(uint_least8_t data)
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
