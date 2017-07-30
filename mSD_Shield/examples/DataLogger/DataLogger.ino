/*
  Data Logger Example
 */

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS1307.h>


DS1307 rtc;


void setup()
{
  int x;

  //init Serial port
  Serial.begin(9600); 
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init RTC
  Serial.print("Init RTC...");
  //only set the date+time one time
  //  rtc.set(0, 0, 8, 24, 12, 2014); //08:00:00 24.12.2014 //sec, min, hour, day, month, year
  rtc.start();
  Serial.println("ok");

  //init SD-Card
  Serial.print("Init SD-Card...");
  if(!SD.begin(4)) //cs-pin=4
  {
    Serial.println("failed");
    while(1);
  }
  else
  {
    Serial.println("ok");
  }

  Serial.println("Start logging...");
}


void loop()
{
  String dataString = ""; //string for logging data
  int sec, min, hour, day, month, year;

  //get time from RTC
  rtc.get(&sec, &min, &hour, &day, &month, &year);
  dataString += String(hour);
  dataString += ":";
  dataString += String(min);
  dataString += ":";
  dataString += String(sec);
  dataString += ";";
  dataString += String(year);
  dataString += "-";
  dataString += String(month);
  dataString += "-";
  dataString += String(day);
  dataString += ";";

  //read analog input 0,1,2
  for(int analogPin=0; analogPin < 3; analogPin++)
  {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if(analogPin < 2)
    {
      dataString += ","; 
    }
  }

  //print data to the serial port
  Serial.println(dataString); 

  //open data file from SD card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if(dataFile) //file opened successfully
  {
    dataFile.println(dataString);
    dataFile.close();
  }  
  else
  {
    Serial.println("Error opening datalog.txt");
  }

  //wait 1 second
  delay(1000);
}
