/*
  digitalWriteFast Example
  For boards with AVR microcontrollers (Arduino Uno, Leonardo, Mega).
  
  Note, the pin parameter has to be a constant.
 */

#include <digitalWriteFast.h>

#define PIN 10

void setup()
{
  volatile byte pin;

  //set pin mode for pin PIN
  pinModeFast(PIN, INPUT);
  pinModeFast(PIN, OUTPUT);

  //set pin state for pin PIN
  digitalWriteFast(PIN, LOW);
  digitalWriteFast(PIN, HIGH);

  //get pin state of pin PIN
  pin = digitalReadFast(PIN);  // save a proper high/low value

  // demonstrate optimization of tests
  if (digitalReadFast(PIN) == HIGH) {
    digitalWriteFast(PIN, LOW);
  }
  if (digitalReadFast(PIN) == LOW) {
    digitalWriteFast(PIN, HIGH);
  }

}


void loop()
{
  //do nothing
}
