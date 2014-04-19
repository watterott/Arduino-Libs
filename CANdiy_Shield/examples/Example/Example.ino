/*
  Example sketch for CANdiy-Shield
 */

#include <SPI.h>
#include <MCP2515.h>


int led2  =  7;
int led3  =  8;
int led13 = 13;
CANMSG msg;
MCP2515 can;


void setup()
{
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led13, OUTPUT);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led13, HIGH);

  Serial.begin(9600); 
  while(!Serial);
  Serial.println("Go...");

  SPI.setClockDivider(SPI_CLOCK_DIV8);

  if(can.initCAN(CAN_BAUD_100K) == 0)
  {
    Serial.println("initCAN() failed");
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led13, LOW);
    while(1);
  }

  if(can.setCANNormalMode(LOW) == 0) //normal mode non single shot
  {
    Serial.println("setCANNormalMode() failed");
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led13, LOW);
    while(1);
  }
}


void loop()
{

  //receiver
  int i = can.receiveCANMessage(&msg, 1000);
  if(i && (msg.data[2] == 123))
  {
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    delay(100);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    delay(100);
    Serial.println("data received");
  }
  
/*
  //transmitter
  msg.adrsValue = 0x7df;
  msg.isExtendedAdrs = false;
  msg.rtr = false;
  msg.dataLength = 8;
  msg.data[0] = 0x02;
  msg.data[1] = 0x01;
  msg.data[2] = 123;
  msg.data[3] = 0;
  msg.data[4] = 0;
  msg.data[5] = 0;
  msg.data[6] = 0;
  msg.data[7] = 0;
  can.transmitCANMessage(msg, 1000);

  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
  delay(100);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
  Serial.println("data transmitted");
  delay(2000);
*/

  digitalWrite(led13, HIGH);
  delay(100);
  digitalWrite(led13, LOW);
  delay(100);
}
