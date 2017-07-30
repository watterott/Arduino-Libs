/*
  RV8523 RTC (Real-Time-Clock) Example

  Uno       A4 (SDA), A5 (SCL)
  Mega      20 (SDA), 21 (SCL)
  Leonardo   2 (SDA),  3 (SCL)
  
  Note: To enable the I2C pull-up resistors on the RTC-Breakout, the jumper J1 has to be closed.
 */

#include <Wire.h>
#include <RV8523.h>


RV8523 rtc;


void setup()
{
  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init RTC
  Serial.println("Init RTC...");

  //set 12 hour mode
  // rtc.set12HourMode();

  //set 24 hour mode
  // rtc.set24HourMode();

  //set the date+time (only one time)
  // rtc.set(0, 0, 8, 24, 12, 2014); //08:00:00 24.12.2014 //sec, min, hour, day, month, year

  //stop/pause RTC
  // rtc.stop();

  //start RTC
  rtc.start();

  //When the power source is removed, the RTC will keep the time.
  rtc.batterySwitchOver(1); //battery switch over on

  //When the power source is removed, the RTC will not keep the time.
  // rtc.batterySwitchOver(0); //battery switch over off
}


void loop()
{
  uint8_t sec, min, hour, day, month;
  uint16_t year;

  //get time from RTC
  rtc.get(&sec, &min, &hour, &day, &month, &year);

  //serial output
  Serial.print("\nTime: ");
  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(min, DEC);
  Serial.print(":");
  Serial.print(sec, DEC);

  Serial.print("\nDate: ");
  Serial.print(day, DEC);
  Serial.print(".");
  Serial.print(month, DEC);
  Serial.print(".");
  Serial.print(year, DEC);

  //wait a second
  delay(1000);
}
