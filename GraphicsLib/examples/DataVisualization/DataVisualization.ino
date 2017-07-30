/*
  Data Visualization Demo

  This sketch reads data from a 1-Wire DS18B20 temperature sensor and I2C TSL45315 light sensor.
 */

#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayUART.h>
#include <DisplayI2C.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
// DisplayUART lcd; //UART (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)


//DS18B20 (1-Wire)
OneWire ds(8); //on pin 8 (4k7 to VCC required)
uint8_t ds_addr[8];

//TSL45315 (I2C)
#define TSL_ADDR     0x29
#define REG_CONTROL  0x00
#define REG_CONFIG   0x01
#define REG_DATALOW  0x04
#define REG_DATAHIGH 0x05
#define REG_ID       0x0A


void setup()
{
  //init Serial port
  Serial.begin(9600); 
  while(!Serial); //wait for serial port to connect - needed for Leonardo only

  //init display
  Serial.println("Init LCD...");
  lcd.begin();
  //SPI Displays
  // lcd.begin(); //spi-clk=Fcpu/4
  // lcd.begin(SPI_CLOCK_DIV2); //spi-clk=Fcpu/2
  // lcd.begin(SPI_CLOCK_DIV4, 8); //spi-clk=Fcpu/4, rst-pin=8
  //UART Displays
  // lcd.begin(5, 6, 7); //rx-pin=5, tx-pin=6, cs-pin=7
  // lcd.begin(5, 6, 7, 8); //rx-pin=5, tx-pin=6, cs-pin=7, rst-pin=8
  //I2C Displays
  // lcd.begin(0x20); //addr=0x20
  // lcd.begin(0x20, 8); //addr=0x20, rst-pin=8

  //init DS18B20
  Serial.println("Init DS18B20...");
  while(ds.search(ds_addr) == 0)
  {
    Serial.println("No DS18B20");
    lcd.drawTextPGM(0, 0, PSTR("No DS18B20"), RGB(0,0,0), RGB(255,255,255), 1);
    ds.reset_search();
    delay(1000);
  }

  //init TSL45315
  Serial.println("Init TSL45315...");
  Wire.begin();
  while(1)
  {
    Wire.beginTransmission(TSL_ADDR);
    Wire.write(0x80|REG_ID);
    Wire.endTransmission();
    Wire.requestFrom(TSL_ADDR, 1); //request 1 byte
    if(Wire.available())
    {
      Serial.print("ID: ");
      while(Wire.available())
      {
        unsigned char c = Wire.read();
        Serial.print(c&0xF0, HEX);
      }
      Serial.println("");
      break;
    }
    else
    {
      Serial.println("No TSL45315");
      lcd.drawTextPGM(0, 0, PSTR("No TSL45315"), RGB(0,0,0), RGB(255,255,255), 1);
      delay(1000);
    }
  }
  Wire.beginTransmission(TSL_ADDR);
  Wire.write(0x80|REG_CONTROL);
  Wire.write(0x03); //power on
  Wire.endTransmission();
  Wire.beginTransmission(TSL_ADDR);
  Wire.write(0x80|REG_CONFIG);
  // Wire.write(0x00); //M=1 T=400ms
  Wire.write(0x01); //M=2 T=200ms
  // Wire.write(0x02); //M=4 T=100ms
  Wire.endTransmission();

  //show default screen
  lcd.fillScreen(RGB(255,255,255));
  lcd.setOrientation(0);
  lcd.drawTextPGM(20, 2, PSTR("Sensor Data"), RGB(0,0,0), RGB(255,255,255), 1);
  lcd.drawTextPGM(20, 12, PSTR("GLCD-Shield"), RGB(220,220,0), RGB(255,255,255), 1);
  lcd.drawTextPGM(12, lcd.getHeight()-10, PSTR("watterott.com"), RGB(150,150,150), RGB(255,255,255), 1);
  lcd.drawLine(0, 22, lcd.getWidth(), 22, RGB(128,128,128));
}


float ds18b20(void) //DS18B20 (code from: http://www.pjrc.com/teensy/td_libs_OneWire.html)
{
  uint8_t data[12];
  float celsius, fahrenheit;

  ds.reset();
  ds.select(ds_addr);
  ds.write(0x44, 1); //start conversion
  delay(1000); //wait 1 second
  ds.reset();
  ds.select(ds_addr);
  ds.write(0xBE); //read
  for(uint8_t i=0; i < 9; i++)
  {
    data[i] = ds.read();
  }

  //Convert the data to actual temperature
  //because the result is a 16 bit signed integer, it should
  //be stored to an "int16_t" type, which is always 16 bits
  //even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  uint8_t cfg = (data[4] & 0x60);
  //at lower res, the low bits are undefined, so let's zero them
       if (cfg == 0x00) raw = raw & ~7;  //9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; //10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; //11 bit res, 375 ms
  //default is 12 bit resolution, 750 ms conversion time

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  
  return celsius;
}


uint32_t tsl45315(void) //TSL45315
{
  uint16_t l, h;
  uint32_t lux;

  Wire.beginTransmission(TSL_ADDR);
  Wire.write(0x80|REG_DATALOW);
  Wire.endTransmission();
  Wire.requestFrom(TSL_ADDR, 2); //request 2 bytes
  l = Wire.read();
  h = Wire.read();
  while(Wire.available()){ Wire.read(); } //received more bytes?
  lux = (h<<8) | (l<<0);
  //lux *= 1; //M=1
  lux *= 2; //M=2
  // lux *= 4; //M=4

  return lux;
}


void loop()
{
  float celsius, fahrenheit;
  uint32_t lux;
  uint8_t h, v, s;

  celsius = ds18b20();
  fahrenheit = celsius * 1.8 + 32.0;

  Serial.print("T = ");
  Serial.print(celsius, 2);
  Serial.print(" C, ");
  Serial.print(fahrenheit, 2);
  Serial.println(" F");

  lcd.setCursor(15, 30);
  lcd.setTextSize(1);
  lcd.setTextColor(RGB(0,0,0), RGB(255,255,255));
  lcd.println("Temperature");
  lcd.setCursor(8, 40);
  lcd.setTextSize(2);
  lcd.setTextColor(RGB(200,0,0), RGB(255,255,255));
  lcd.print(celsius);
  lcd.println(" C");
  lcd.setCursor(8, 60);
  lcd.setTextColor(RGB(0,200,0), RGB(255,255,255));
  lcd.print(fahrenheit);
  lcd.print(" F");

  lux = tsl45315();

  Serial.print("Lux = ");
  Serial.println(lux, DEC);

  lcd.setCursor(15, 85);
  lcd.setTextSize(1);
  lcd.setTextColor(RGB(0,0,0), RGB(255,255,255));
  lcd.println("Light (lux)");
  lcd.setCursor(8, 95);
  lcd.setTextSize(2);
  lcd.setTextColor(RGB(0,0,200), RGB(255,255,255));
  lcd.print(lux);
  lcd.print("   ");
}
