/*
  Read/Write File Example
 */

#include <Wire.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>
#include <SD.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)

File myFile;


void setup()
{
  int x;

  //init Serial port
  Serial.begin(9600); 
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init LCD
  Serial.println("Init Display...");
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.fillScreen(RGB(255,255,255));

  //init SD-Card
  Serial.println("Init SD-Card...");
  x = lcd.drawText(5, 5, "Init SD-Card...", RGB(0,0,0), RGB(255,255,255), 1);
  if(!SD.begin(4)) //cs-pin=4
  {
    Serial.println("failed");
    lcd.drawText(x, 5, "failed", RGB(0,0,0), RGB(255,255,255), 1);
    while(1);
  }

  //open file for writing
  Serial.println("Open File...");
  x = lcd.drawText(5, 5, "Open File...", RGB(0,0,0), RGB(255,255,255), 1);
  myFile = SD.open("test.txt", FILE_WRITE);
  if(myFile)
  {
    Serial.println("Writing...");
    lcd.drawText(5, 5, "Writing...", RGB(0,0,0), RGB(255,255,255), 1);
    myFile.println("This is a Test: ABC 123");
    myFile.close();
  }
  else
  {
    Serial.println("error");
    lcd.drawText(x, 5, "error", RGB(0,0,0), RGB(255,255,255), 1);
  }

  //open file for reading
  Serial.println("Open File...");
  x = lcd.drawText(5, 5, "Open File...", RGB(0,0,0), RGB(255,255,255), 1);
  myFile = SD.open("test.txt");
  if(myFile)
  {
    Serial.println("Reading...");
    lcd.drawText(x, 5, "Reading...", RGB(0,0,0), RGB(255,255,255), 1);
    lcd.setCursor(0, 20);
    while(myFile.available())
    {
      uint8_t c;
      c = myFile.read();
      Serial.write(c);
      lcd.print((char)c);
    }
    myFile.close();
  }
  else
  {
    Serial.println("error");
    lcd.drawText(x, 5, "error", RGB(0,0,0), RGB(255,255,255), 1);
  }
}


void loop()
{
  //do nothing
}
