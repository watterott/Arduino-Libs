/*
  Infrastructure Test

  This sketch connects two RedFly-Shields using an infrastructure network
  and synchronizes the state of a switch connected to pin A5 and GND.
  One shield is the server and the other one is the client.
  Use the Serial Monitor for configuration (9600 Baud).
 */

#include <RedFly.h>
#include <RedFlyClient.h>
#include <RedFlyServer.h>


uint8_t led_pin = 13; //pin 13 has a LED connected on most Arduino boards
uint8_t sw_pin = A5; //connect switch to pin A5 and GND

uint8_t working_mode = 0; //mode: 0=server, 1=client
uint16_t comm_port = 1234; //communication port
uint32_t last_time = 0;
uint8_t last_state = 0;

byte ip_server[] = { 192,168,0,50 }; //ip from server shield
byte ip_client[] = { 192,168,0,51 }; //ip from client shield


RedFlyClient client(ip_server, comm_port); //create client class
RedFlyServer server(comm_port); //create server class


//debug output functions (9600 Baud, 8N2)
//Leonardo boards use USB for communication, so we dont need to disable the RedFly
void debugout(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.print(s);
#else
  RedFly.disable();
  Serial.print(s);
  RedFly.enable();
#endif
}

void debugoutln(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.println(s);
#else
  RedFly.disable();
  Serial.println(s);
  RedFly.enable();
#endif
}


void setup()
{
  uint8_t ret;

  //set pin config
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(sw_pin, INPUT);
  digitalWrite(sw_pin, HIGH); //pull-up on

#if defined(__AVR_ATmega32U4__) //Leonardo boards use USB for communication
  Serial.begin(9600); //init serial port and set baudrate
  while(!Serial); //wait for serial port to connect (needed for Leonardo only)
#endif

  ret = RedFly.init(); //9600 baud, HIGH_POWER
  if(ret)
  {
    debugoutln("INIT ERR"); //there are problems with the communication between the Arduino and the RedFly
    for(;;); //do nothing forevermore
  }
  else
  {
    RedFly.disable();
    Serial.println("Press 0 for server or 1 for client");
    while(Serial.available() == 0);
    working_mode = Serial.read() - '0';
    while(Serial.available() != 0){ Serial.read(); }
    if(working_mode == 0) //server
    {
      Serial.println("Server...");
    }
    else
    {
      Serial.println("Client...");
    }
    RedFly.enable();

    do
    {
      delay(1000);
      ret = RedFly.join("wlan-ssid", "wlan-passw", INFRASTRUCTURE);
    }
    while(ret != 0);

    if(working_mode == 0) //server
    {
      ret = RedFly.begin(ip_server); //use static ip
    }
    else //client
    {
      ret = RedFly.begin(ip_client); //use static ip
    }

    if(ret)
    {
      debugoutln("BEGIN ERR");
      RedFly.disconnect();
      for(;;); //do nothing forevermore
    }
    else
    {
      debugoutln("started");
    }
  }
}


void loop()
{
  uint8_t c;
  unsigned long ms = millis();

  if(working_mode == 0) //server
  {
    if(!server.connected()) //server port open?
    {
      server.stop();
      server.begin();
    }
    else if(server.available()) //new data available
    {
      c = (server.read()!=0) ? HIGH : LOW;
      digitalWrite(led_pin, c);
    }
    else if(((ms-last_time) > 100)) //check switch every 100ms
    {
      last_time = ms; //save time
      c = digitalRead(sw_pin); //read switch
      if(c != last_state) //switch state changed?
      {
        last_state = c;
        server.write(c); //send data over WLAN
      }
    }
  }
  else //client
  {
    if(!client.connected()) //client connected to server?
    {
      client.stop();
      client.connect();
    }
    else if(client.available()) //new data available
    {
      c = (client.read()!=0) ? HIGH : LOW;
      digitalWrite(led_pin, c);
    }
    else if(((ms-last_time) > 100)) //check switch every 100ms
    {
      last_time = ms; //save time
      c = digitalRead(sw_pin); //read switch
      if(c != last_state) //switch state changed?
      {
        last_state = c;
        client.write(c); //send data over WLAN
      }
    }
  }
}
