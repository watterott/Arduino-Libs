/*
  Frequency to Voltage Converter

  Frequency-Input - D8/PB0/ICP1

  DAC8760
  CLR   - connected to GND
  LATCH - D10
  DIN   - D11/MOSI
  SDO   - D12/MISO
  SCLK  - D13/CLK
 */

#include <util/delay.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <DAC8760.h>


//#define DEBUG //enable serial debug output (9600 baud)

#define INPUT_PIN 8 //D8/PB0/ICP1 (frequency input pin)

//F_CPU = 16000000 Hz = 62.5ns
//t = 800.....16000 (*62.5ns)
//f = 20kHz...1kHz  (f = 1/((1/F_CPU)*t))
//v = 10V.....0V
//n = 65535...0
#define F_MIN  1000 // 1000Hz, 250...100000Hz (min. frequency -> 0V)
#define F_MAX 20000 //20000Hz, 250...100000Hz (max. frequency -> 10V)
#define N_MIN     0 // 0V = 0
#define N_MAX 65535 //10V = 65535


DAC8760 dac;

volatile uint_least16_t t_avg=0;
uint_least8_t t_over=0;


ISR(TIMER1_OVF_vect) //overflow after about 4ms
{
  t_over = 1;
  t_avg  = F_CPU/F_MIN; //set to min. freq.
}


ISR(TIMER1_CAPT_vect) //input capture irq
{
  uint_least16_t t;
  static uint_least16_t t_last=0;

  TCNT1 = 0x0000; //reset timer
  t = ICR1; //get input capture value

  if(t_over) //overflow?
  {
    t_over = 0;
    t_last = 0;
  }
  else
  {
    //choose only on calculation method

    //1 - set avg to t
     t_avg = t;

    //2 - make avg from last and current
    // t_avg   = t_last + t;
    // t_avg >>= 1; //div by 2

    //3 - make avg from all and current
    // t_avg  += t;
    // t_avg >>= 1; //div by 2

    t_last = t;
  }
}


void print(char *s)
{
#ifdef DEBUG
  while(*s)
  {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = *s++;
  }
#endif
}


void print(uint_least16_t i)
{
#ifdef DEBUG
  char tmp[8];
  
  sprintf(tmp, "%u", i);
  print(tmp);
#endif
}


void setup()
{
  //init Serial port
#ifdef DEBUG
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only
#endif

  //init Pins
  print("Init Pins...\n");
  pinMode(INPUT_PIN, INPUT);
  digitalWrite(INPUT_PIN, HIGH); //pull-up

  //init DAC
  print("Init DAC...\n");
  dac.begin();
  dac.write(0);
  
  //disable Arduino interrupts and init timer
  print("Init IRQs...\n");
  cli(); //disable all interrupts
  //disable UART interrupts
  UCSR0B &= ~((1<<RXCIE0) | (1<<TXCIE0));
  //reset presaclers
  GTCCR = (1<<PSRASY); 
  //disable timer 0 (8 bit)
  TCNT0  = 0;
  TIMSK0 = 0;
  TCCR0A = 0x00;
  TCCR0B = 0x00;
  //enable timer 1 (16 bit)
  TCNT1  = 0x0000;
  TIMSK1 = (1<<ICIE1) | (1<<TOIE1);
  TCCR1A = 0x00;
  TCCR1B = (1<<ICNC1) | (1<<ICES1) | (0<<CS12) | (0<<CS11) | (1<<CS10); //ICP1 rising, clk=F_CPU/1
  TCCR1C = 0x00;
  //disable timer 2 (8 bit)
  TCNT2  = 0x00;
  TIMSK2 = 0x00;
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  sei(); //enable all interrupts
}


void loop()
{
  uint_least16_t t, n;
  static uint_least16_t t_last=0, n_last=0;
  uint_least32_t f;
  uint8_t oldSREG;

  oldSREG = SREG;
  cli();
  t = t_avg;
  SREG = oldSREG;

  if(t != t_last) //t changed?
  {
    t_last = t;

    f = F_CPU/(uint_least32_t)t; //calculate f from t
         if(f < F_MIN){ f = F_MIN; } //freq. to low?
    else if(f > F_MAX){ f = F_MAX; } //freq. to high?
    n = map(f, F_MIN, F_MAX, N_MIN, N_MAX); //map freq. to voltage

    if(n != n_last) //n changed?
    {
      n_last = n;

      //set DAC output
      dac.write(n); //set DAC output

#ifdef DEBUG
      //print("t:");
      //print(t);
      print(" f:");
      print(f);
      //print(" n:");
      //print(n);
      print(" mV:");
      n = map(n, 0, 65535, 0, 10000); //map n to voltage
      print(n);
      print("\n");
#endif
    }
  }
}
