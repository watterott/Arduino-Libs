/*
  GraphicsLib Demo

  This sketch demonstrates the functions of the GraphicsLib.
 */

#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <SSD1331.h>
#include <S65L2F50.h>
#include <S65LPH88.h>
#include <S65LS020.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayUART.h>
#include <DisplayI2C.h>


//Declare only one display !
// SSD1331 lcd;
// S65L2F50 lcd;
// S65LPH88 lcd;
// S65LS020 lcd;
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
// DisplayUART lcd; //UART (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)


void setup()
{
  uint8_t clear_bg=0x00; //0x80 = dont clear background for fonts (only for DisplayXXX)

  //init display
  lcd.begin();
  //SPI Displays
  // lcd.begin(); //spi-clk=Fcpu/4
  // lcd.begin(SPI_CLOCK_DIV2); //spi-clk=Fcpu/2
  // lcd.begin(SPI_CLOCK_DIV4, 8); //spi-clk=Fcpu/4, rst-pin=8
  //UART Displays
  // lcd.begin(5, 6, 7); //rx-pin=5, tx-pin=6, cs-pin=7
  // lcd.begin(5, 6, 7, 8); //rx-pin=5, tx-pin=6, cs-pin=7, rst-pin=8
  //I2C Displays
  // lcd.begin(0x20); //addr=0x20
  // lcd.begin(0x20, 8); //addr=0x20, rst-pin=8


  //clear screen
  lcd.fillScreen(RGB(255,255,255));


  //setOrientation
  lcd.setOrientation(0);
  lcd.drawText(10, 10, "test 1", RGB(0,0,0), RGB(255,255,255), 1);
  lcd.setOrientation(90);
  lcd.drawText(10, 10, "test 2", RGB(0,0,0), RGB(255,255,255), 1);
  lcd.setOrientation(180);
  lcd.drawText(10, 10, "test 3", RGB(0,0,0), RGB(255,255,255), 1);
  lcd.setOrientation(270);
  lcd.drawText(10, 10, "test 4", RGB(0,0,0), RGB(255,255,255), 1);
  lcd.setOrientation(0);


  //drawPixel, drawLine, drawRect, fillRect, drawTriangle, fillTriangle, drawRoundRect, fillRoundRect, drawCircle, fillCircle, drawEllipse, fillEllipse
  lcd.fillEllipse(20,30,10,15,RGB(200,  0,  0));
  lcd.drawEllipse(20,30,10,15,RGB(  0,  0,  0));

  lcd.fillCircle(50,30,10,RGB(  0,200,  0));
  lcd.drawCircle(50,30,12,RGB(  0,  0,  0));

  lcd.fillRoundRect(71,21,18,18,5,RGB(  0,  0,200));
  lcd.drawRoundRect(70,20,20,20,5,RGB(  0,  0,  0));

  lcd.fillTriangle(30,50,10,90,50,80,RGB(  0,200,200));
  lcd.drawTriangle(30,50,10,90,50,80,RGB(  0,  0,  0));

  lcd.fillRect(50,50,20,20,RGB(200,200,  0));
  lcd.drawRect(51,51,18,18,RGB(  0,  0,  0));

  lcd.drawLine(10,10,50,60,RGB( 10, 10, 10));
  lcd.drawLine(10,60,50,10,RGB( 10, 10, 10));

  lcd.drawPixel(5,5,RGB(100,100,100));


  //drawText
  lcd.drawTextPGM(50, 10, PSTR("String from Flash"), RGB(0,0,0), RGB(255,255,255), 1|clear_bg); //string from flash (only AVR)
  lcd.drawText(50, 20, "String from RAM", RGB(0,0,0), RGB(255,255,255), 1|clear_bg);            //string from ram


  //drawInteger
  lcd.drawInteger(5, 20, 1234, DEC, RGB(0,0,0), RGB(255,255,255), 1|clear_bg); //dec
  lcd.drawInteger(5, 30, 1234, HEX, RGB(0,0,0), RGB(255,255,255), 1|clear_bg); //hex
  lcd.drawInteger(5, 40, 1234, OCT, RGB(0,0,0), RGB(255,255,255), 1|clear_bg); //oct
  lcd.drawInteger(5, 50, 1234, BIN, RGB(0,0,0), RGB(255,255,255), 1|clear_bg); //bin


  //print
  lcd.setTextSize(1|clear_bg);
  lcd.setTextColor(RGB(200,0,0), RGB(255,255,255));
  lcd.setCursor(2, 60);
  lcd.print("String from RAM\nnext line1\n"); //string from ram
  lcd.println(12345);      //int
  lcd.println('A');        //char
  lcd.println(123456789L); //long
  lcd.println(98765.43);   //float/double
}


void loop()
{
  //do nothing
}
