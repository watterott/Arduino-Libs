/*
  Paint Demonstration
 */

#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)



void writeCalData(void)
{
  uint16_t i, addr=0;
  uint8_t *ptr;

  EEPROM.write(addr++, 0xAA);
  
  ptr = (uint8_t*)&lcd.tp_matrix;
  for(i=0; i < sizeof(CAL_MATRIX); i++)
  {
    EEPROM.write(addr++, *ptr++);
  }

  return;
}


uint8_t readCalData(void)
{
  uint16_t i, addr=0;
  uint8_t *ptr;
  uint8_t c;

  c = EEPROM.read(addr++);
  if(c == 0xAA)
  {
    ptr = (uint8_t*)&lcd.tp_matrix;
    for(i=0; i < sizeof(CAL_MATRIX); i++)
    {
      *ptr++ = EEPROM.read(addr++);
    }
    return 0;
  }

  return 1;
}


void setup()
{
  //init display
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8

  //set touchpanel calibration data
  lcd.touchRead();
  if(lcd.touchZ() || readCalData()) //calibration data in EEPROM?
  {
    lcd.touchStartCal(); //calibrate touchpanel
    writeCalData(); //write data to EEPROM
  }

  //clear screen
  lcd.fillScreen(RGB(255,255,255));

  //show backlight power and cal text
  lcd.led(50); //backlight 0...100%
  lcd.drawText(2, 2, "BL    ", RGB(255,0,0), RGB(255,255,255), 1);
  lcd.drawInteger(20, 2, 50, DEC, RGB(255,0,0), RGB(255,255,255), 1);
  lcd.drawText(lcd.getWidth()-30, 2, "CAL", RGB(255,0,0), RGB(255,255,255), 1);
}


void loop()
{
  char tmp[128];
  static uint16_t last_x=0, last_y=0;
  static uint8_t led=60;
  unsigned long m;
  static unsigned long prevMillis=0;

  //service routine for touch panel
  lcd.touchRead();

  if(lcd.touchZ()) //touch press?
  {
    //change backlight power
    if((lcd.touchX() < 45) && (lcd.touchY() < 15))
    {
      m = millis();
      if((m - prevMillis) > 800) //change only every 800ms
      {
        prevMillis = m;

        led += 10;
        if(led > 100)
        {
          led = 10;
        }
        lcd.led(led);
        lcd.drawText(2, 2, "BL    ", RGB(255,0,0), RGB(255,255,255), 1);
        lcd.drawInteger(20, 2, led, DEC, RGB(255,0,0), RGB(255,255,255), 1);
        lcd.drawText(lcd.getWidth()-30, 2, "CAL", RGB(255,0,0), RGB(255,255,255), 1);
      }
    }

    //calibrate touch panel
    else if((lcd.touchX() > (lcd.getWidth()-30)) && (lcd.touchY() < 15))
    {
      lcd.touchStartCal();
      writeCalData();
      lcd.drawText(2, 2, "BL    ", RGB(255,0,0), RGB(255,255,255), 1);
      lcd.drawInteger(20, 2, led, DEC, RGB(255,0,0), RGB(255,255,255), 1);
      lcd.drawText(lcd.getWidth()-30, 2, "CAL", RGB(255,0,0), RGB(255,255,255), 1);
    }

    //draw line
    else if((last_x != lcd.touchX()) || (last_y != lcd.touchY()))
    {
      sprintf(tmp, "X:%03i Y:%03i P:%03i", lcd.touchX(), lcd.touchY(), lcd.touchZ());
      lcd.drawText(50, 2, tmp, RGB(0,0,0), RGB(255,255,255), 1);

      if(last_x == 0)
      {
        lcd.drawPixel(lcd.touchX(), lcd.touchY(), RGB(0,0,0));
      }
      else
      {
        lcd.drawLine(last_x, last_y, lcd.touchX(), lcd.touchY(), RGB(0,0,0));
      }
      last_x = lcd.touchX();
      last_y = lcd.touchY();
    }
  }
  else
  {
    last_x = 0;
  }
}
