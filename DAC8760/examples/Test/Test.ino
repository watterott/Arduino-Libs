/*
  DAC8760 DAC Test

  CLR   - connected to GND
  LATCH - D10
  DIN   - D11/MOSI
  SDO   - D12/MISO
  SCLK  - D13/CLK
 */

#include <SPI.h>
#include <digitalWriteFast.h>
#include <DAC8760.h>


DAC8760 dac;


void setup()
{
  //init Serial port
  Serial.begin(9600); 
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init DAC
  Serial.println("Init DAC...");
  dac.begin();

  dac.write(0); //0 32767 65535
}


void loop()
{
  //do nothing
}
