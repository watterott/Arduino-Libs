/*
  ADS1147 ADC Example

  CS  - D10
  SDI - D11/MOSI
  SDO - D12/MISO
  CLK - D13/CLK
 */

#include <SPI.h>
#include <digitalWriteFast.h>
#include <ADS1147.h>


ADS1147 adc;


void setup()
{
  int32_t i;

  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init DAC
  Serial.println("Init...");
  adc.begin(20, 4, 1000); //20 SPS, Gain 4, 1000 uA

  i = adc.read(0x01); //read 0+1
  Serial.println(i, DEC);
  i = adc.read(0x23); //read 2+3
  Serial.println(i, DEC);
}


void loop()
{
  //do nothing
}
