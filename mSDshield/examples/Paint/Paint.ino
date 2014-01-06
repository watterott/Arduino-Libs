/*
  Paint Demonstration
 */

#include <SPI.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <ADS7846.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
MI0283QT9 lcd;  //MI0283QT9 Adapter v1

ADS7846 tp;

#define TP_EEPROMADDR (0x00) //eeprom address for calibration data


void setup()
{
  //init display
  lcd.begin();
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8
  lcd.fillScreen(RGB(255,255,255));

  //init touch controller
  tp.begin();

  //touch-panel calibration
  tp.service();
  if(tp.getPressure() > 5)
  {
    tp.doCalibration(&lcd, TP_EEPROMADDR, 0); //dont check EEPROM for calibration data
  }
  else
  {
    tp.doCalibration(&lcd, TP_EEPROMADDR, 1); //check EEPROM for calibration data
  }

  //clear screen
  lcd.fillScreen(RGB(255,255,255));

  //show backlight power and cal text
  lcd.led(50); //backlight 0...100%
  lcd.drawText( 2,                2, "BL 50 ", RGB(255,0,0), RGB(255,255,255), 1);
  lcd.drawText(lcd.getWidth()-30, 2, "CAL",    RGB(255,0,0), RGB(255,255,255), 1);
}


void loop()
{
  char tmp[128];
  static uint16_t last_x=0, last_y=0;
  static uint8_t led=60;
  unsigned long m;
  static unsigned long prevMillis=0;

  //service routine for touch panel
  tp.service();

  //show tp data
  sprintf(tmp, "X:%03i|%04i Y:%03i|%04i P:%03i", tp.getX(), tp.getXraw(), tp.getY(), tp.getYraw(), tp.getPressure());
  lcd.drawText(45, 2, tmp, RGB(0,0,0), RGB(255,255,255), 1);

  if(tp.getPressure() > 3) //touch press?
  {
    //change backlight power
    if((tp.getX() < 45) && (tp.getY() < 15))
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
      }
    }

    //calibrate touch panel
    else if((tp.getX() > (lcd.getWidth()-30)) && (tp.getY() < 15))
    {
      tp.doCalibration(&lcd, TP_EEPROMADDR, 0);
    }

    //draw line
    else
    {
      if(last_x == 0)
      {
        lcd.drawPixel(tp.getX(), tp.getY(), RGB(0,0,0));
      }
      else
      {
        lcd.drawLine(last_x, last_y, tp.getX(), tp.getY(), RGB(0,0,0));
      }
      last_x = tp.getX();
      last_y = tp.getY();
    }

    //show backlight power and cal text
    lcd.drawText( 2,                2, "BL    ", RGB(255,0,0), RGB(255,255,255), 1);
    lcd.drawText(20,                2, led,      RGB(255,0,0), RGB(255,255,255), 1);
    lcd.drawText(lcd.getWidth()-30, 2, "CAL",    RGB(255,0,0), RGB(255,255,255), 1);
  }
  else
  {
    last_x = 0;
  }
}
