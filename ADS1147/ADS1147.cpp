#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
#endif
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "../SPI/SPI.h"
#include "../digitalWriteFast/digitalWriteFast.h"
#include "ADS1147.h"


// #define SOFTWARE_SPI


#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define CS_PIN         (10) //10 or A3
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
       defined(__AVR_ATmega644P__))    //--- Arduino 644 ---

# define CS_PIN         (10) //10 or A3
# define MOSI_PIN       (5)
# define MISO_PIN       (6)
# define CLK_PIN        (7)

#elif defined(__AVR_ATmega32U4__)      //--- Arduino Leonardo ---

# define CS_PIN         (10) //10 or A3
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

# define CS_PIN         (10) //10 or A3
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


//Commands
#define CMD_WAKEUP      0x01 //exit sleep mode
#define CMD_SLEEP       0x02 //enter sleep mode
#define CMD_SYNC        0x04 //synchronize the A/D conversion
#define CMD_RESET       0x06 //reset to power-up values
#define CMD_NOP         0xFF //no operation
#define CMD_RDATA       0x12 //read data once
#define CMD_RDATAC      0x14 //read data continuously
#define CMD_SDATAC      0x16 //stop reading data continuously
#define CMD_RREG        0x20 //read from register
#define CMD_WREG        0x40 //write to register
#define CMD_SYSOCAL     0x60 //system offset calibration
#define CMD_SYSGCAL     0x61 //system gain calibration
#define CMD_SELFOCAL    0x62 //self offset calibration

//Registers
#define REG_MUX0        0x00
#define REG_VBIAS       0x01
#define REG_MUX1        0x02
#define REG_SYS0        0x03
#define REG_OFC0        0x04
#define REG_OFC1        0x05
#define REG_OFC2        0x06
#define REG_FSC0        0x07
#define REG_FSC1        0x08
#define REG_FSC2        0x09
#define REG_IDAC0       0x0A
#define REG_IDAC1       0x0B
#define REG_GPIOCFG     0x0C
#define REG_GPIODIR     0x0D
#define REG_GPIODAT     0x0E

//Multiplexer Control Register 0
#define MUX0_SN0        (0<<0)
#define MUX0_SN1        (1<<0)
#define MUX0_SN2        (2<<0)
#define MUX0_SN3        (3<<0)
#define MUX0_SP0        (0<<3)
#define MUX0_SP1        (1<<3)
#define MUX0_SP2        (2<<3)
#define MUX0_SP3        (3<<3)
#define MUX0_BCSOFF     (0<<6)
#define MUX0_BCS05UA    (1<<6)
#define MUX0_BCS2UA     (2<<6)
#define MUX0_BCS10UA    (3<<6)

//Bias Voltage Register
#define VBIAS_OFF       (0x00)
#define VBIAS_AIN0      (1<<0)
#define VBIAS_AIN1      (1<<1)
#define VBIAS_AIN1      (1<<2)
#define VBIAS_AIN1      (1<<3)

//Multiplexer Control Register 1
#define MUX1_CALNORMAL  (0<<0) //normal operation, PGA gain set by SYS0 
#define MUX1_CALOFFSET  (1<<0) //offset measurement, PGA gain set by SYS0 
#define MUX1_CALGAIN    (2<<0) //gain measurement, PGA gain 1
#define MUX1_CALTEMP    (3<<0) //temperature measurement, PGA gain 1
#define MUX1_CALREF1    (4<<0) //ext. REF1 measurement, PGA gain 1
#define MUX1_CALREF0    (5<<0) //ext. REF0 measurement, PGA gain 1
#define MUX1_CALAVDD    (6<<0) //AVDD measurement, PGA gain 1
#define MUX1_CALDVDD    (7<<0) //DVDD measurement, PGA gain 1
#define MUX1_REF0       (0<<3) //REF0 input pair selected
#define MUX1_REF1       (1<<3) //REF1 input pair selected
#define MUX1_REFOB      (2<<3) //0nboard reference selected
#define MUX1_REF0OB     (3<<3) //0nboard reference selected and internally connected to REF0 input pair
#define MUX1_INTREFOFF  (0<<5) //internal reference is always off
#define MUX1_INTREFON   (1<<5) //internal reference is always on
#define MUX1_INTREFONC  (2<<5) //internal reference is on when a conversion is in progress
#define MUX1_INTOSC     (0<<7) //internal oscillator
#define MUX1_EXTOSC     (0<<7) //external oscillator

//System Control Register 0
#define SYS0_DOR5       (0<<0) //   5 SPS
#define SYS0_DOR10      (1<<0) //  10 SPS
#define SYS0_DOR20      (2<<0) //  20 SPS
#define SYS0_DOR40      (3<<0) //  40 SPS
#define SYS0_DOR80      (4<<0) //  80 SPS
#define SYS0_DOR160     (5<<0) // 160 SPS
#define SYS0_DOR320     (6<<0) // 320 SPS
#define SYS0_DOR640     (7<<0) // 640 SPS
#define SYS0_DOR1000    (8<<0) //1000 SPS
#define SYS0_DOR2000    (9<<0) //2000 SPS
#define SYS0_PGA1       (0<<4)
#define SYS0_PGA2       (1<<4)
#define SYS0_PGA4       (2<<4)
#define SYS0_PGA8       (3<<4)
#define SYS0_PGA16      (4<<4)
#define SYS0_PGA32      (5<<4)
#define SYS0_PGA64      (6<<4)
#define SYS0_PGA128     (7<<4)

//IDAC Control Register 0
#define IDAC0_MAGOFF    (0<<0)
#define IDAC0_MAG50UA   (1<<0)
#define IDAC0_MAG100UA  (2<<0)
#define IDAC0_MAG250UA  (3<<0)
#define IDAC0_MAG500UA  (4<<0)
#define IDAC0_MAG750UA  (5<<0)
#define IDAC0_MAG1000UA (6<<0)
#define IDAC0_MAG1500UA (7<<0)
#define IDAC0_DOUT      (0<<3) //DOUT/DRDY pin -> Data Out
#define IDAC0_DOUTDRDY  (1<<3) //DOUT/DRDY pin -> Data Out and Data Ready (active low)

//IDAC Control Register 1
#define IDAC1_I2DIR0    (0<<0)
#define IDAC1_I2DIR1    (1<<0)
#define IDAC1_I2DIR2    (2<<0)
#define IDAC1_I2DIR3    (3<<0)
#define IDAC1_I2DIROFF  (15<<0)
#define IDAC1_I1DIR0    (0<<4)
#define IDAC1_I1DIR1    (1<<4)
#define IDAC1_I1DIR2    (2<<4)
#define IDAC1_I1DIR3    (3<<4)
#define IDAC1_I1DIROFF  (15<<4)


//-------------------- Constructor --------------------


ADS1147::ADS1147(void)
{
  return;
}


//-------------------- Public --------------------


uint_least8_t ADS1147::begin(uint_least8_t drate, uint_least8_t gain, uint_least16_t current)
{
  uint_least8_t r;

  //init pins
  pinMode(CS_PIN, OUTPUT);
  CS_DISABLE();
  pinMode(CLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  //digitalWrite(MISO_PIN, HIGH); //pull-up

  //init SPI
#if !defined(SOFTWARE_SPI)
  SPI.begin();
#endif

  //init ADS
  switch(drate)
  {
    case    5: drate = SYS0_DOR5;    break;
    case   10: drate = SYS0_DOR10;   break;
    case   20: drate = SYS0_DOR20;   break;
    default:
    case   40: drate = SYS0_DOR40;   break;
    case   80: drate = SYS0_DOR80;   break;
    case  160: drate = SYS0_DOR160;  break;
    case  320: drate = SYS0_DOR320;  break;
    case  640: drate = SYS0_DOR640;  break;
    case 1000: drate = SYS0_DOR1000; break;
    case 2000: drate = SYS0_DOR2000; break;
  }

  switch(gain)
  {
    default:
    case   1: gain = SYS0_PGA1;   break;
    case   2: gain = SYS0_PGA2;   break;
    case   4: gain = SYS0_PGA4;   break;
    case   8: gain = SYS0_PGA8;   break;
    case  16: gain = SYS0_PGA16;  break;
    case  32: gain = SYS0_PGA32;  break;
    case  64: gain = SYS0_PGA64;  break;
    case 128: gain = SYS0_PGA128; break;
  }

  switch(current)
  {
    default:
    case    0: current = IDAC0_MAGOFF;    break;
    case   50: current = IDAC0_MAG50UA;   break;
    case  100: current = IDAC0_MAG100UA;  break;
    case  250: current = IDAC0_MAG250UA;  break;
    case  500: current = IDAC0_MAG500UA;  break;
    case  750: current = IDAC0_MAG750UA;  break;
    case 1000: current = IDAC0_MAG1000UA; break;
    case 1500: current = IDAC0_MAG1500UA; break;
  }

  wr_cmd(CMD_RESET);
  wr_cmd(CMD_SELFOCAL);
  r = (rd_reg(REG_IDAC0)&0xF0)>>4; //read revision
  wr_reg(REG_MUX0, MUX0_BCSOFF | MUX0_SP0 | MUX0_SN1);
  wr_reg(REG_VBIAS, VBIAS_OFF);
  wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
  wr_reg(REG_SYS0, gain | drate);
  wr_reg(REG_IDAC0, IDAC0_DOUTDRDY | current);
  wr_reg(REG_IDAC1, IDAC1_I1DIROFF | IDAC1_I2DIROFF);

  return r;
}


uint_least8_t ADS1147::begin(void)
{
  return begin(0, 0, 0);
}


int_least16_t ADS1147::read(uint_least8_t chn)
{
  int_least16_t data=0;

  switch(chn)
  {
    case 0x01:
      wr_reg(REG_IDAC1, IDAC1_I1DIR0 | IDAC1_I2DIR1);
      wr_reg(REG_MUX0, MUX0_SP0 | MUX0_SN1);
      data = rd_data();
      wr_reg(REG_IDAC1, IDAC1_I1DIROFF | IDAC1_I2DIROFF);
      break;
    case 0x23:
      wr_reg(REG_IDAC1, IDAC1_I1DIR2 | IDAC1_I2DIR3);
      wr_reg(REG_MUX0, MUX0_SP2 | MUX0_SN3);
      data = rd_data();
      wr_reg(REG_IDAC1, IDAC1_I1DIROFF | IDAC1_I2DIROFF);
      break;

    case 0xF9: //offset measurement
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALOFFSET);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      break;
    case 0xFA: //gain measurement
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALGAIN);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      break;
    case 0xFB: //temperature measurement
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALTEMP);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      break;
    case 0xFC: //REF1 measurement
      wr_reg(REG_IDAC1, IDAC1_I1DIR0 | IDAC1_I2DIR1);
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALREF1);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      wr_reg(REG_IDAC1, IDAC1_I1DIROFF | IDAC1_I2DIROFF);
      break;
    case 0xFD: //REF0 measurement
      wr_reg(REG_IDAC1, IDAC1_I1DIR0 | IDAC1_I2DIR1);
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALREF0);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      wr_reg(REG_IDAC1, IDAC1_I1DIROFF | IDAC1_I2DIROFF);
      break;
    case 0xFE: //AVDD measurement
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALAVDD);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      break;
    case 0xFF: //DVDD measurement
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REFOB | MUX1_CALDVDD);
      data = rd_data();
      wr_reg(REG_MUX1, MUX1_INTOSC | MUX1_INTREFON | MUX1_REF0 | MUX1_CALNORMAL);
      break;
  }

  return data;
}


//-------------------- Private --------------------


int_least16_t ADS1147::rd_data(void)
{
  int_least32_t d1, d2;

  //set SPI settings
#if !defined(SOFTWARE_SPI)
# if defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# endif
# if F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV16);
# elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# elif F_CPU >= 4000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# endif
  SPI.setDataMode(SPI_MODE1); //SPI_MODE1
#endif

  wr_cmd(CMD_SYNC);

  CS_ENABLE();

  while(MISO_READ() != 0); //wait till data sampled
  wr_spi(CMD_RDATA);
  d1 = rd_spi();
  d1 <<= 8;
  d1 |= rd_spi();
  rd_spi(); //NOP, to let DRDY go high

  while(MISO_READ() != 0); //wait till data sampled
  wr_spi(CMD_RDATA);
  d2 = rd_spi();
  d2 <<= 8;
  d2 |= rd_spi();
  rd_spi(); //NOP, to let DRDY go high
  
  CS_DISABLE();

  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif

  return int_least16_t((d1+d2)/2UL);
}


void ADS1147::wr_cmd(uint_least8_t cmd)
{
  //set SPI settings
#if !defined(SOFTWARE_SPI)
# if defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# endif
# if F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV16);
# elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# elif F_CPU >= 4000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# endif
  SPI.setDataMode(SPI_MODE1); //SPI_MODE1
#endif

  CS_ENABLE();

  wr_spi(cmd);
  if(cmd == CMD_SYNC)
  {
    wr_spi(cmd);
  }

  CS_DISABLE();

  if(cmd == CMD_RESET)
  {
    delay(1); //_delay_ms(1);
  }
  else if(cmd == CMD_SELFOCAL)
  {
    delay(10); //_delay_ms(10);
  }

  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif

  return;
}


uint_least8_t ADS1147::rd_reg(uint_least8_t reg)
{
  uint_least8_t data;

  //set SPI settings
#if !defined(SOFTWARE_SPI)
# if defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# endif
# if F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV16);
# elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# elif F_CPU >= 4000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# endif
  SPI.setDataMode(SPI_MODE1); //SPI_MODE1
#endif

  CS_ENABLE();

  wr_spi(CMD_RREG | reg);
  wr_spi(0);
  data = rd_spi();

  CS_DISABLE();

  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif

  return data;
}


void ADS1147::wr_reg(uint_least8_t reg, uint_least8_t data)
{
  //set SPI settings
#if !defined(SOFTWARE_SPI)
# if defined(__AVR__)
  uint_least8_t spcr, spsr;
  spcr = SPCR;
  spsr = SPSR;
# endif
# if F_CPU >= 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV16);
# elif F_CPU >= 8000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
# elif F_CPU >= 4000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV4);
# endif
  SPI.setDataMode(SPI_MODE1); //SPI_MODE1
#endif

  CS_ENABLE();

  wr_spi(CMD_WREG | reg);
  wr_spi(0);
  wr_spi(data);

  delayMicroseconds(1);
  CS_DISABLE();

  //restore SPI settings
#if !defined(SOFTWARE_SPI) && defined(__AVR__)
  SPCR = spcr;
  SPSR = spsr;
#endif

  return;
}


uint_least8_t ADS1147::rd_spi(void)
{
#if defined(SOFTWARE_SPI)
  uint_least8_t data=0;
  MOSI_HIGH();
  for(uint_least8_t bit=8; bit!=0; bit--)
  {
    CLK_HIGH();
    CLK_LOW();
    data <<= 1;
    if(MISO_READ())
    {
      data |= 1;
    }
    else
    {
      //data |= 0;
    }
  }
  return data;
#else
  return SPI.transfer(0xFF);
#endif
}


void ADS1147::wr_spi(uint_least8_t data)
{
#if defined(SOFTWARE_SPI)
  for(uint_least8_t mask=0x80; mask!=0; mask>>=1)
  {
    CLK_HIGH();
    if(mask & data)
    {
      MOSI_HIGH();
    }
    else
    {
      MOSI_LOW();
    }
    CLK_LOW();
  }
#else
  SPI.transfer(data);
#endif
  return;
}
