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
#include "S65LPH88.h"


//#define SOFTWARE_SPI //use software SPI on pins 11,12,13

#ifndef LCD_WIDTH
# define LCD_WIDTH      (176)
# define LCD_HEIGHT     (132)
#endif


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define RST_PIN        (57) //A3=57
# define CS_PIN         (56) //A2=56
# if defined(SOFTWARE_SPI)
#  define MOSI_PIN      (11)
#  define MISO_PIN      (12)
#  define CLK_PIN       (13)
# else
#  define MOSI_PIN      (51)
#  define MISO_PIN      (50)
#  define CLK_PIN       (52)
# endif

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define RST_PIN        (17) //A3=17
# define CS_PIN         (16) //A2=16
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

# define RST_PIN        (17) //A3=17
# define CS_PIN         (16) //A2=16
# define MOSI_PIN       (11)
# define MISO_PIN       (12)
# define CLK_PIN        (13)

#endif


#define RST_DISABLE()   digitalWriteFast(RST_PIN, HIGH)
#define RST_ENABLE()    digitalWriteFast(RST_PIN, LOW)

#define CS_DISABLE()    digitalWriteFast(CS_PIN, HIGH)
#define CS_ENABLE()     digitalWriteFast(CS_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define MISO_READ()     digitalReadFast(MISO_PIN)

#define CLK_HIGH()      digitalWriteFast(CLK_PIN, HIGH)
#define CLK_LOW()       digitalWriteFast(CLK_PIN, LOW)


//-------------------- Constructor --------------------


S65LPH88::S65LPH88(void) : GraphicsLib(LCD_WIDTH, LCD_HEIGHT)
{
  return;
}


//-------------------- Public --------------------


void S65LPH88::begin(uint_least8_t clock_div)
{
  //init pins
#if defined(RST_PIN)
  pinMode(RST_PIN, OUTPUT);
  RST_ENABLE();
#endif
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  CS_DISABLE();

#if !defined(SOFTWARE_SPI)
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(clock_div);
  SPI.begin();
#endif

  //reset display
  reset(clock_div);

  return;
}


void S65LPH88::begin(void)
{
  return begin(SPI_CLOCK_DIV4);
}


void S65LPH88::led(uint_least8_t power)
{
  return;
}


void S65LPH88::setOrientation(uint_least16_t o)
{
  uint_least16_t p;

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
    p = 0x0008; //entry mode --
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
    p = 0x0038; //entry mode ++
  }

  if(p != 0)
  {
    wr_cmd(0x05, p);
  }

  setArea(0, 0, lcd_width-1, lcd_height-1);

  return;
}


void S65LPH88::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  switch(lcd_orientation)
  {
    default:
    case 0:
      wr_cmd(0x16, (y1<<8)|(y0)); //set y
      wr_cmd(0x17, (x1<<8)|(x0)); //set x
      wr_cmd(0x21, (x0<<8)|(y0)); //set cursor pos
      break;
    //case 90:
      //not implemented
    //  break;
    case 180:
      wr_cmd(0x16, (((LCD_HEIGHT-1)-y0)<<8)|(((LCD_HEIGHT-1)-y1))); //set y
      wr_cmd(0x17, (((LCD_WIDTH-1)-x0)<<8)|(((LCD_WIDTH-1)-x1)));   //set x
      wr_cmd(0x21, (((LCD_WIDTH-1)-x0)<<8)|(((LCD_HEIGHT-1)-y0)));  //set cursor pos
      break;
    //case 270:
      //not implemented
    //  break;
  }


  return;
}


void S65LPH88::drawStart(void)
{
  wr_reg(0x22);
  CS_ENABLE();
  wr_spi(0x76);

  return;
}


void S65LPH88::draw(uint_least16_t color)
{
  wr_spi(color>>8);
  wr_spi(color);

  return;
}


void S65LPH88::drawStop(void)
{
  CS_DISABLE();

  return;
}


//-------------------- Private --------------------


#if defined(__AVR__)
//const prog_uint8_t initdataLPH88[] = 
const uint8_t initdataLPH88[] PROGMEM = 
#else
const uint8_t initdataLPH88[] = 
#endif
{
  //display off
  0x40| 3, 0x07, 0x00, 0x00,
  0xC0|10, //10ms
  //power on sequence
  0x40| 3, 0x02, 0x04, 0x00, //lcd drive control
  0x40| 3, 0x0C, 0x00, 0x01, //power control 3: VC        //step 1
  0x40| 3, 0x0D, 0x00, 0x06, //power control 4: VRH
  0x40| 3, 0x04, 0x00, 0x00, //power control 2: CAD
  0x40| 3, 0x0D, 0x06, 0x16, //power control 4: VRL
  0x40| 3, 0x0E, 0x00, 0x10, //power control 5: VCM
  0x40| 3, 0x0E, 0x10, 0x10, //power control 5: VDV
  0x40| 3, 0x03, 0x00, 0x00, //power control 1: BT        //step 2
  0x40| 3, 0x03, 0x00, 0x00, //power control 1: DC
  0x40| 3, 0x03, 0x00, 0x0C, //power control 1: AP
  0xC0|40, //40ms
  0x40| 3, 0x0E, 0x2D, 0x1F, //power control 5: VCOMG     //step 3
  0xC0|40, //40ms
  0x40| 3, 0x0D, 0x06, 0x16, //power control 4: PON       //step 4
  0xC0|50, //50ms
  0xC0|50, //50ms
  //display options
  0x40| 3, 0x05, 0x00, 0x38, //entry mode ++
  0x40| 3, 0x16, (LCD_HEIGHT-1)<<8, 0, //set y
  0x40| 3, 0x17, (LCD_WIDTH-1)<<8, 0, //set x
  0x40| 3, 0x21, 0<<8, 0, //set cursor pos
  //display on sequence (bit2 = reversed colors)
  0x40| 3, 0x07, 0x00, 0x05, //display control: D0
  0x40| 3, 0x07, 0x00, 0x25, //display control: GON
  0x40| 3, 0x07, 0x00, 0x27, //display control: D1
  0x40| 3, 0x07, 0x00, 0x37, //display control: DTE
  0xC0|10, //10ms
  0xFF   , 0xFF
};


void S65LPH88::reset(uint_least8_t clock_div)
{
  uint_least8_t c, d, e, i;
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
  ptr = &initdataLPH88[0];
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
        for(i=c&0x3F; i!=0; i-=3)
        {
          c = pgm_read_byte(ptr++);
          d = pgm_read_byte(ptr++);
          e = pgm_read_byte(ptr++);
          wr_cmd(c, (d<<8)|(e<<0));
        }
        break;
      case 0xC0: //delay
        c = c&0x3F;
        delay(c);
        break;
    }
  }

  //restore SPI settings
#if !defined(SOFTWARE_SPI)
  SPI.setClockDivider(clock_div);
#endif

  //clear display buffer
  fillScreen(0);

  return;
}


void S65LPH88::wr_cmd(uint_least8_t cmd, uint_least16_t param)
{
  wr_reg(cmd);

  CS_ENABLE();
  wr_spi(0x76); //instruction or RAM data
  wr_spi(param>>8);
  wr_spi(param>>0);
  CS_DISABLE();

  return;
}


void S65LPH88::wr_reg(uint_least8_t reg)
{
  CS_ENABLE();
  wr_spi(0x74); //index register
  wr_spi(0x00);
  wr_spi(reg);
  CS_DISABLE();

  return;
}


void S65LPH88::wr_spi(uint_least8_t data)
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
