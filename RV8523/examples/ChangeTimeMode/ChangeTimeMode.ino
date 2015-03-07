/*
  RV8523 RTC (Real-Time-Clock) Example

  Uno       A4 (SDA), A5 (SCL)
  Mega      20 (SDA), 21 (SCL)
  Leonardo   2 (SDA),  3 (SCL)
 */

#include <Wire.h>
#include <RV8523.h>


RV8523 rtc;


void setup()
{
  //init Serial port
  Serial.begin(115200);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init RTC
  Serial.println("Init RTC...");

  // if module has a battery, you can switch to battery switch over
  // if power source of your device is removed, the rtc will keep the time
  rtc.batterySwitchOverOn();
  
  // battery switch over mode can be removed by calling the following function or by removing the battery 
  // rtc.batterySwitchOverOff();
  
  rtc.set(55, 59, 11, 7, 3, 2015); //08:00:00 24.12.2014 //sec, min, hour, day, month, year

  //start RTC
  rtc.start();
}


void loop()
{
  uint8_t sec, min, hour, day, month;
  uint16_t year;

  //get time from RTC
  rtc.get(&sec, &min, &hour, &day, &month, &year);
  
  
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
  Serial.println(year, DEC);
  

  //wait a second
  delay(1000);
}
