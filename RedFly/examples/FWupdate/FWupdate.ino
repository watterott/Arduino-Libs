/*
  Firmware Update

  Sketch that should be loaded on a Firmare Update.
 */

#define RX_PIN   (0)
#define TX_PIN   (1)
#define RST_PIN  (2)
#define CS_PIN   (3)


void setup()
{
  pinMode(RX_PIN, INPUT);

  pinMode(TX_PIN, INPUT);

  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH); //reset off

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); //disable

  while(1);
}


void loop()
{
}
