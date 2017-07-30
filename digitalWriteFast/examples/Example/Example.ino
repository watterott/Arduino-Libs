/*
  digitalWriteFast Example
  For boards with AVR microcontrollers (Arduino Uno, Leonardo, Mega).
  
  Note, the pin parameter has to be a constant.
 */

#include <digitalWriteFast.h>


void setup()
{
  byte pin;

  //set pin mode for pin 10
  pinModeFast(10, INPUT);
  pinModeFast(10, OUTPUT);

  //set pin state for pin 10
  digitalWriteFast(10, LOW);
  digitalWriteFast(10, HIGH);

  //get pin state of pin 10
  pin = digitalReadFast(10);
}


void loop()
{
  //do nothing
}
