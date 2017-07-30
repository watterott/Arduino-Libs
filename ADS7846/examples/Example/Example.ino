/*
  ADS7846 Touch-Controller Example
 */

#include <SPI.h>
#include <digitalWriteFast.h>
//#include <GraphicsLib.h>
//#include <MI0283QT2.h>
//#include <MI0283QT9.h>
#include <ADS7846.h>


ADS7846 ads;


void setup()
{
  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init Touch-Controller
  Serial.println("Init...");
  ads.begin();
}


void loop()
{
  ads.service();

  uint16_t x = ads.getXraw();
  uint16_t y = ads.getYraw();
  uint16_t z = ads.getPressure();
  if(z)
  {
    Serial.println(x, DEC);
    Serial.println(y, DEC);
    Serial.println(z, DEC);
  }

  delay(100);
}
