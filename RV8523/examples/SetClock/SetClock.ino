/*
  RV8523 RTC (Real-Time-Clock) Set Clock Example

  Uno       A4 (SDA), A5 (SCL)
  Mega      20 (SDA), 21 (SCL)
  Leonardo   2 (SDA),  3 (SCL)

  Note: To enable the I2C pull-up resistors on the RTC-Breakout, the jumper J1 has to be closed.

  This sketch allows to set the time using the serial console. On linux use a command like:

    stty -F /dev/ttyUSB1 speed 9600
    date '+T%H%M%S%d%m%Y' > /dev/ttyUSB1

  to read the current local time from the serial console. Or just type a string like

    T23595924122015

  to set the clock to 23:59:59 24th of December 2015 using Arduinos/Genuinos serial monitor.

  Type an arbitary string not beginning with 'T' to show the current time.
 */

#include <Wire.h>
#include <RV8523.h>


#define BUFF_MAX 32

RV8523 rtc;
unsigned int recv_size = 0;
char recv[BUFF_MAX];


void setup()
{
  //init Serial port
  Serial.begin(9600);
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init RTC
  Serial.println("Init RTC...");
 
  //set 24 hour mode
  rtc.set24HourMode();
 
  //start RTC
  rtc.start();

  //When the power source is removed, the RTC will keep the time.
  rtc.batterySwitchOver(1); //battery switch over on
}


void loop()
{
  if (Serial.available() > 0) {
    setClock();
  }
}


void setClock()
{
  char in;

  in = Serial.read();
  Serial.print(in); 

  if((in == 10 || in == 13) && (recv_size > 0))
  {
    parseCmd(recv, recv_size);
    printTime();
    recv_size = 0;
    recv[0] = 0;
    return;
  }
  else if (in < 48 || in > 122) //ignore ~[0-9A-Za-z]
  {
    //do nothing
  }
  else if (recv_size > BUFF_MAX - 2) //drop lines that are too long
  {
    recv_size = 0;
    recv[0] = 0;
  }
  else if (recv_size < BUFF_MAX - 2)
  {
    recv[recv_size] = in;
    recv[recv_size + 1] = 0;
    recv_size += 1;
  }
}


// Parse the time string and set the clock accordingly
void parseCmd(char *cmd, int cmdsize)
{
  uint8_t i;
  uint8_t reg_val;
  char buff[BUFF_MAX];

  //ThhmmssDDMMYYYY aka set time
  if (cmd[0] == 84 && cmdsize == 15)
  {
    rtc.set(
      inp2toi(cmd, 5),
      inp2toi(cmd, 3),
      inp2toi(cmd, 1),
      inp2toi(cmd, 7),
      inp2toi(cmd, 9),
      inp2toi(cmd, 11) * 100 + inp2toi(cmd, 13)
    ); //sec, min, hour, day, month, year
    Serial.println("OK");
  }
}


// just output the time
void printTime()
{
  uint8_t sec, min, hour, day, month;
  uint16_t year;
  
  //get time from RTC
  rtc.get(&sec, &min, &hour, &day, &month, &year);

  //serial output
  Serial.print("\nTime: ");
  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(min, DEC);
  Serial.print(":");
  Serial.print(sec, DEC);

  Serial.print("\nDate: ");
  Serial.print(day, DEC);
  Serial.print(".");
  Serial.print(month, DEC);
  Serial.print(".");
  Serial.print(year, DEC);
}


uint8_t inp2toi(char *cmd, const uint16_t seek)
{
  uint8_t rv;
  rv = (cmd[seek] - 48) * 10 + cmd[seek + 1] - 48;
  return rv;
}
