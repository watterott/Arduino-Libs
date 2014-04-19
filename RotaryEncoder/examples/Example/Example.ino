/*
  Rotary Encoder Example
  
  The pins can be changed in the file RotaryEncoder.cpp
 */

#include <RotaryEncoder.h>


RotaryEncoder encoder;


ISR(TIMER2_OVF_vect) //timer 2 overflow interrupt
{
  TCNT2 -= 250; //1000 Hz
  
  encoder.service();
}


void setup()
{
  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init Rotary Encoder
  Serial.println("Init Rotary Encoder...");
  encoder.init();

  //init Timer2
  TCCR2B  = (1<<CS22); //clk=F_CPU/64
  TCNT2   = 0x00;
  TIMSK2 |= (1<<TOIE2); //enable overflow interupt

  //interrupts on
  sei();
}


void loop()
{
  static int sw=0, pos=0, last_pos=0;
  
  sw = encoder.sw();
  pos += encoder.step();

  if(sw || (pos != last_pos))
  {
    last_pos = pos;
    Serial.print("\nEnc: ");
    Serial.print(sw, DEC);
    Serial.print(" ");
    Serial.print(pos, DEC);
  }
}
