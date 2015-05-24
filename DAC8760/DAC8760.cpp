/*
  DAC8760 DAC Lib for Arduino
  by Watterott electronic (www.watterott.com)
 */

#include <inttypes.h>
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
# include <avr/io.h>
#endif
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "SPI.h"
#include "digitalWriteFast.h"
#include "DAC8760.h"


//#define SOFTWARE_SPI //use software SPI on pins MOSI:11, MISO:12, SCK:13

#if defined(SOFTWARE_SPI)
# define LATCH_PIN      10 //SPI_SW_SS_PIN
# define MOSI_PIN       SPI_SW_MOSI_PIN
# define MISO_PIN       SPI_SW_MISO_PIN
# define SCK_PIN        SPI_SW_SCK_PIN
#else
# define LATCH_PIN      10 //SPI_HW_SS_PIN
# define MOSI_PIN       SPI_HW_MOSI_PIN
# define MISO_PIN       SPI_HW_MISO_PIN
# define SCK_PIN        SPI_HW_SCK_PIN
#endif

#define LATCH_HIGH()    digitalWriteFast(LATCH_PIN, HIGH)
#define LATCH_LOW()     digitalWriteFast(LATCH_PIN, LOW)

#define MOSI_HIGH()     digitalWriteFast(MOSI_PIN, HIGH)
#define MOSI_LOW()      digitalWriteFast(MOSI_PIN, LOW)

#define MISO_READ()     digitalReadFast(MISO_PIN)

#define SCK_HIGH()      digitalWriteFast(SCK_PIN, HIGH)
#define SCK_LOW()       digitalWriteFast(SCK_PIN, LOW)


//Address Byte
#define ADDR_NOP        0x00 //No operation (NOP)
#define ADDR_DATA       0x01 //Write DAC data register
#define ADDR_READ       0x02 //Register read
#define ADDR_CTRL       0x55 //Write control register
#define ADDR_RESET      0x56 //Write reset register
#define ADDR_CONFIG     0x57 //Write configuration register
#define ADDR_GAIN       0x58 //Write DAC gain calibration register
#define ADDR_ZERO       0x59 //Write DAC zero calibration register
#define ADDR_WDT        0x95 //Watchdog timer reset

//CTRL Bits
#define CTRL_CLRSEL     15 //VOUT clear value: 0 = VOUT is 0V, 1 = VOUT is midscale in unipolar output and negative-full-scale in bipolar output
#define CTRL_OVR        14 //Setting the bit increases the voltage output range by 10%.
#define CTRL_REXT       13 //External current setting resistor enable.
#define CTRL_OUTEN      12 //0 = output is disabled, 1 = output is determined by RANGE bits
#define CTRL_SRCLK       8 //Slew rate clock control. Ignored when bit SREN = 0
#define CTRL_SRSTEP      5 //Slew rate step size control. Ignored when bit SREN = 0
#define CTRL_SREN        4 //Slew Rate Enable.
#define CTRL_DCEN        3 //Daisy-chain enable.
#define CTRL_RANGE       0 //Output range bits.


//-------------------- Constructor --------------------


DAC8760::DAC8760(void)
{
  return;
}


//-------------------- Public --------------------


void DAC8760::begin(void)
{
  uint_least8_t r;

  //init pins
  pinMode(LATCH_PIN, OUTPUT);
  LATCH_LOW();
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  //digitalWrite(MISO_PIN, HIGH); //pull-up

  //init SPI
#if !defined(SOFTWARE_SPI)
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4); //SPI_CLOCK_DIV4
  SPI.setDataMode(SPI_MODE0); //SPI_MODE0
  //SPI.setBitOrder(MSBFIRST);
#endif

  wr_reg(ADDR_RESET, 0x01); //software reset
  wr_reg(ADDR_DATA, 0x0000); //set data to 0
  wr_reg(ADDR_CTRL, (0x01<<CTRL_RANGE)|(0<<CTRL_OVR)|(1<<CTRL_OUTEN)); //0-10V

  return;
}


void DAC8760::write(uint_least16_t val)
{
  wr_reg(ADDR_DATA, val);

  return;
}


//-------------------- Private --------------------


void DAC8760::wr_reg(uint_least8_t reg, uint_least16_t data)
{
/*
  //set SPI settings
#if !defined(SOFTWARE_SPI)
# if defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# endif
  SPI.setClockDivider(SPI_CLOCK_DIV4); //SPI_CLOCK_DIV4
  SPI.setDataMode(SPI_MODE0); //SPI_MODE0
  //SPI.setBitOrder(MSBFIRST);
#endif
*/
  wr_spi(reg);
  wr_spi(data>>8);
  wr_spi(data>>0);

  LATCH_HIGH();
  LATCH_LOW();
/*
  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif
*/
  return;
}


void DAC8760::wr_spi(uint_least8_t data)
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
