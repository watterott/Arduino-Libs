/*
  RV8523 RTC Lib for Arduino
  by Watterott electronic (www.watterott.com)
  
  Update: 2015-03-07 - Mario Lukas - added battery switch over support
                                   - added 12/24 hour switching support

 */

#include <inttypes.h>
#if defined(__AVR__)
# include <avr/io.h>
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "Wire.h"
#include "RV8523.h"


#define I2C_ADDR (0xD0>>1)


//-------------------- Constructor --------------------


RV8523::RV8523(void)
{
  Wire.begin();

  return;
}


//-------------------- Public --------------------


void RV8523::start(void)
{
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x00));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if(val & (1<<5))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x00));
    Wire.write(val & ~(1<<5)); //clear STOP (bit 5)
    Wire.endTransmission();
  }

  return;
}


void RV8523::stop(void)
{
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x00));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if(!(val & (1<<5)))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x00));
    Wire.write(val | (1<<5)); //set STOP (bit 5)
    Wire.endTransmission();
  }

  return;
}


/**
* set 12 Hour Mode
*/
void RV8523::set12HourMode(){
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x00));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if(!(val & (1<<3)))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x00));
    Wire.write(val | (1<<3)); //set 12 Hour Mode (bit 3)
    Wire.endTransmission();
  }

  return;
}


/**
* set 24 Hour Mode
*/
void RV8523::set24HourMode(){
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x00));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if((val & (1<<3)))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x00));
    Wire.write(val & ~(1<<3)); //set 24 Hour Mode (bit 3) to 0
    Wire.endTransmission();
  }

  return;
}


/**
* activate battery switch over mode
*/
void RV8523::batterySwitchOverOn()
{   
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x02));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if((val & (1<<6)))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x02));
    Wire.write(val & ~(1<<6)); 
    Wire.endTransmission();
  }

  return;
}


/**
* deactivate battery switch over mode
*/
void RV8523::batterySwitchOverOff()
{   
  uint8_t val;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x00));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1);
  val = Wire.read();

  if(!(val | (1<<6)))
  {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(byte(0x00));
    Wire.write(val | (1<<6)); 
    Wire.endTransmission();
  }

  return;
}



void RV8523::get(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *month, uint16_t *year)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x03));
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 7);
  *sec   = bcd2bin(Wire.read() & 0x7F);
  *min   = bcd2bin(Wire.read() & 0x7F);
  *hour  = bcd2bin(Wire.read() & 0x3F); //24 hour mode
  *day   = bcd2bin(Wire.read() & 0x3F);
           bcd2bin(Wire.read() & 0x07); //day of week
  *month = bcd2bin(Wire.read() & 0x1F);
  *year  = bcd2bin(Wire.read()) + 2000;

  return;
}


void RV8523::get(int *sec, int *min, int *hour, int *day, int *month, int *year)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x03));
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 7);
  *sec   = bcd2bin(Wire.read() & 0x7F);
  *min   = bcd2bin(Wire.read() & 0x7F);
  *hour  = bcd2bin(Wire.read() & 0x3F); //24 hour mode
  *day   = bcd2bin(Wire.read() & 0x3F);
           bcd2bin(Wire.read() & 0x07); //day of week
  *month = bcd2bin(Wire.read() & 0x1F);
  *year  = bcd2bin(Wire.read()) + 2000;

  return;
}


void RV8523::set(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
{
  if(year > 2000)
  {
    year -= 2000;
  }

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(byte(0x03));
  Wire.write(bin2bcd(sec));
  Wire.write(bin2bcd(min));
  Wire.write(bin2bcd(hour));
  Wire.write(bin2bcd(day));
  Wire.write(bin2bcd(0));
  Wire.write(bin2bcd(month));
  Wire.write(bin2bcd(year));
  Wire.endTransmission();

  return;
}


void RV8523::set(int sec, int min, int hour, int day, int month, int year)
{
  return set((uint8_t)sec, (uint8_t)min, (uint8_t)hour, (uint8_t)day, (uint8_t)month, (uint16_t)year);
}


//-------------------- Private --------------------


uint8_t RV8523::bin2bcd(uint8_t val)
{
  return val + 6 * (val / 10);
}


uint8_t RV8523::bcd2bin(uint8_t val)
{
  return val - 6 * (val >> 4);
}
