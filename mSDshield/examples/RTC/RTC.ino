/*
  RTC (Real-Time-Clock) Example
 */

#include <Wire.h>
#include <SPI.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h> 
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>
#include <DS1307.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)

DS1307 rtc;


void setup()
{
  //init Serial port
  Serial.begin(9600); 
  while(!Serial); //wait for serial port to connect - needed for Leonardo only
  
  //init Display
  Serial.println("Init Display...");
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.fillScreen(RGB(255,255,255));

  //init RTC
  Serial.println("Init RTC...");
  //only set the date+time one time
  //  rtc.set(0, 0, 8, 24, 12, 2014); //08:00:00 24.12.2014 //sec, min, hour, day, month, year
  rtc.start();
}


void loop()
{
  uint8_t sec, min, hour, day, month;
  uint16_t year;
  char buf[16];
  
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

  //display output
  sprintf(buf, "%02i : %02i : %02i", hour, min, sec);
  lcd.drawText(10, 5, buf, RGB(0,0,0), RGB(255,255,255), 1);

  sprintf(buf, "%02i . %02i . %04i", day, month, year);
  lcd.drawText(10, 20, buf, RGB(0,0,0), RGB(255,255,255), 1);

  //wait a second
  delay(1000);
}
