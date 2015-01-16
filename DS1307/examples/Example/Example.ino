/*
  DS1307 RTC (Real-Time-Clock) Example

  Uno       A4 (SDA), A5 (SCL)
  Mega      20 (SDA), 21 (SCL)
  Leonardo   2 (SDA),  3 (SCL)
 */

#include <Wire.h>
#include <DS1307.h>


DS1307 rtc;


void setup()
{
  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init RTC
  Serial.println("Init RTC...");

  //only set the date+time one time
  // rtc.set(0, 0, 8, 24, 12, 2014); //08:00:00 24.12.2014 //sec, min, hour, day, month, year

  //stop/pause RTC
  // rtc.stop();

  //start RTC
  rtc.start();
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
