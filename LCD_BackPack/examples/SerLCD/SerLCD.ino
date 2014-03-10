/*
  SerLCD Firmware for LCD-BackPack
 */

#include <digitalWriteFast.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>


#define VERSION  "0.01" //v0.01

#define LCD_EN   2  //enable
#define LCD_RS   A2 //command/data
#define LCD_D4   3  //data bit 4
#define LCD_D5   4  //data bit 5
#define LCD_D6   5  //data bit 6
#define LCD_D7   6  //data bit 7
#define LCD_VE   A1 //contrast
#define BL_RED   7  //blacklight red
#define BL_GREEN 8  //blacklight green
#define BL_BLUE  9  //blacklight blue


LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//settings
uint16_t eeprom_addr=0; //start address to store settings in eeprom
uint8_t eeprom_magic=0xAA, splash_screen=1, baudrate=2;
uint8_t lcd_width=16, lcd_lines=2, cursor_pos=0, line_pos=0;
uint8_t contrast=10, bl_on=1, bl_red=50, bl_green=0, bl_blue=0, bl_rgb=1;
char splash_line[2][21];


ISR(TIMER2_OVF_vect)
{
  static uint8_t c=1, r=1, g=1, b=1;

  //TCNT2 -= 250;

  //contrast
  if(--c == 0)
  {
    if(digitalReadFast(LCD_VE))
    {
      if(contrast < 32)
      {
        c = 32-contrast;
        digitalWriteFast(LCD_VE, LOW);
      }
    }
    else if(contrast)
    {
      c = contrast;
      digitalWriteFast(LCD_VE, HIGH);
    }
  }

  //backlight
  if(bl_on)
  {
    if(bl_red)
    {
      if(--r == 0)
      {
        if(digitalReadFast(BL_RED))
        {
          if(bl_red < 100)
          {
            r = 100-bl_red;
            digitalWriteFast(BL_RED, LOW);
          }
        }
        else
        {
          r = bl_red;
          digitalWriteFast(BL_RED, HIGH);
        }
      }
    }
    if(bl_green)
    {
      if(--g == 0)
      {
        if(digitalReadFast(BL_GREEN))
        {
          if(bl_green < 100)
          {
            g = 100-bl_green;
            digitalWriteFast(BL_GREEN, LOW);
          }
        }
        else
        {
          g = bl_green;
          digitalWriteFast(BL_GREEN, HIGH);
        }
      }
    }
    if(bl_blue)
    {
      if(--b == 0)
      {
        if(digitalReadFast(BL_BLUE))
        {
          if(bl_blue < 100)
          {
            b = 100-bl_blue;
            digitalWriteFast(BL_BLUE, LOW);
          }
        }
        else
        {
          b = bl_blue;
          digitalWriteFast(BL_BLUE, HIGH);
        }
      }
    }
  }
}


void read_settings(void)
{
  uint16_t i, addr=eeprom_addr;
  uint8_t *ptr;

  if(EEPROM.read(eeprom_addr+0) != 0xAA)  //magic
  {
    EEPROM.write(addr++, eeprom_magic); //eeprom_addr+0
    EEPROM.write(addr++, splash_screen);
    EEPROM.write(addr++, baudrate);
    EEPROM.write(addr++, lcd_width);
    EEPROM.write(addr++, lcd_lines);
    EEPROM.write(addr++, contrast);
    EEPROM.write(addr++, bl_on);
    EEPROM.write(addr++, bl_red);
    EEPROM.write(addr++, bl_green);
    EEPROM.write(addr++, bl_blue);
    EEPROM.write(addr++, bl_rgb); //eeprom_addr+10
    splash_line[0][0] = 0;
    for(i=0; i < sizeof(splash_line); i++)
    {
      EEPROM.write(addr++, 0);
    }
  }
  else
  {
    addr++; //eeprom_magic  = EEPROM.read(addr++);
    splash_screen = EEPROM.read(addr++); //eeprom_addr+0
    baudrate      = EEPROM.read(addr++);
    lcd_width     = EEPROM.read(addr++);
    lcd_lines     = EEPROM.read(addr++);
    contrast      = EEPROM.read(addr++);
    bl_on         = EEPROM.read(addr++);
    bl_red        = EEPROM.read(addr++);
    bl_green      = EEPROM.read(addr++);
    bl_blue       = EEPROM.read(addr++);
    bl_rgb        = EEPROM.read(addr++); //eeprom_addr+10
    if(bl_rgb == 0)
    {
      bl_green = 0;
      bl_blue  = 0;
    }
    ptr = (uint8_t*)&splash_line[0][0];
    for(i=0; i < sizeof(splash_line); i++)
    {
      *ptr++ = EEPROM.read(addr++);
    }
  }
  
  if((splash_line[0][0] == 0x00) || (splash_line[0][0] == 0xFF))
  {
    splash_line[0][0]  = 0;
    splash_line[1][0]  = 0;
    splash_line[0][20] = 0;
    splash_line[1][20] = 0;
  }
}


void set_baudrate(uint8_t i)
{
  uint8_t b;
  uint32_t br;

  switch(i)
  {
    case 0: b=i; br=  2400; break;
    case 1: b=i; br=  4800; break;
    default:
    case 2: b=i; br=  9600; break;
    case 3: b=i; br= 14400; break;
    case 4: b=i; br= 19200; break;
    case 5: b=i; br= 38400; break;
    case 6: b=i; br= 57600; break;
    case 7: b=i; br=115200; break;
    case 8: b=i; br= 28800; break;
  }

  if(baudrate != b)
  {
    baudrate = b;
    EEPROM.write(eeprom_addr+2, baudrate);
  }
  Serial.end();
  Serial.begin(br);
}


void set_backlight(uint8_t on)
{
  if((on == 0) && (bl_on != 0))
  {
    bl_on = 0;
    EEPROM.write(eeprom_addr+6, bl_on);
    digitalWriteFast(BL_RED, LOW);
    digitalWriteFast(BL_GREEN, LOW);
    digitalWriteFast(BL_BLUE, LOW);
  }
  else if((on != 0) && (bl_on == 0))
  {
    bl_on = 1;
    EEPROM.write(eeprom_addr+6, bl_on);
  }
}


void setup()
{
  //init pins
  pinMode(LCD_EN, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_D4, OUTPUT);
  pinMode(LCD_D5, OUTPUT);
  pinMode(LCD_D6, OUTPUT);
  pinMode(LCD_D7, OUTPUT);
  pinMode(LCD_VE, OUTPUT);
  pinMode(BL_RED, OUTPUT);
  pinMode(BL_GREEN, OUTPUT);
  pinMode(BL_BLUE, OUTPUT);
  digitalWriteFast(LCD_VE, LOW);
  digitalWriteFast(BL_RED, LOW);
  digitalWriteFast(BL_GREEN, LOW);
  digitalWriteFast(BL_BLUE, LOW);

  //init serial port
  Serial.begin(9600);

  //load settings
  read_settings();

  //init timer2 for contast and backlight
  TCNT2  = 0;
  TCCR2B  = (1<<CS20); //prescale set to 1
  //TCCR2B = (1<<CS21); //prescale set to 8
  TIMSK2 = (1<<TOIE2);

  //init lcd
  lcd.begin(lcd_width, lcd_lines);
  if(splash_screen)
  {
    if((splash_line[0][0] == 0x00) || (splash_line[0][0] == 0xFF)) //no custom splash screen
    {
      strcpy_P(splash_line[0], PSTR("  SerLCD v"VERSION""));
      strcpy_P(splash_line[1], PSTR(" watterott.com"));
      splash_screen = 0;
    }
    lcd.setCursor(0, 0);
    lcd.print(splash_line[0]);
    lcd.setCursor(0, 1);
    lcd.print(splash_line[1]);
    if(splash_screen == 0) //no custom splash screen
    {
      splash_screen = 1;
      splash_line[0][0] = 0;
      splash_line[1][0] = 0;
    }
    delay(500);
  }
  delay(500);
  lcd.setCursor(0, 0);
  lcd.clear();

  if(Serial.available()) //serial data available?
  {
    if(Serial.read() == 0x12) //0x12 = 18
    {
      set_baudrate(2); //2 -> 9600 baud
      lcd.print("Reset to 9600");
      lcd.setCursor(0, 0);
    }
  }

  //init serial port
  set_baudrate(baudrate);
}


void loop()
{
  uint8_t c, *ptr;

  if(Serial.available() == 0) //new serial data available?
  {
    return;
  }

  c = Serial.read();

  if(c == 0xFE) //0xFE = 254 special lcd command
  {
    while(Serial.available() == 0);
    c = Serial.read();

    if((c>>4) != 3) //if not 0b.0000.0011, then send it to lcd
    {
      lcd.command(c);
      if(c == 0x01) //clear lcd
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        cursor_pos = 0;
        line_pos = 0;
      }
      else if (c & 0x80) //move cursor
      {
        cursor_pos = c & 0x7F; //ignore first bit - obtain address
      }
    }
  }
  else if(c == 0x7C) //0x7C = 124 super special lcd command
  {
    while(Serial.available() == 0);
    c = Serial.read();

    if(c == 0x01) //backlight 100%
    {
      set_backlight(1);
      if((bl_rgb == 0) && (bl_red == 0))
      {
        bl_red   = 100;
        bl_green = 0;
        bl_blue  = 0;
      }
      else if((bl_rgb != 0) && (bl_red == 0) && (bl_green == 0) && (bl_blue == 0))
      {
        bl_red   = 100;
        bl_green = 100;
        bl_blue  = 100;
      }
    }
    else if((c == 0x02) || (c == 0x80)) //backlight off
    {
      set_backlight(0);
    }
    else if(c == 0x03) //lcd type
    {
      if(lcd_width != 20)
      {
        lcd_width = 20;
        EEPROM.write(eeprom_addr+2, lcd_width);
      }
    }
    else if(c == 0x04) //lcd type
    {
      if(lcd_width != 16)
      {
        lcd_width = 16;
        EEPROM.write(eeprom_addr+2, lcd_width);
      }
    }
    else if(c == 0x05) //lcd lines
    {
      if(lcd_lines != 4)
      {
        lcd_lines = 4;
        EEPROM.write(eeprom_addr+3, lcd_lines);
      }
    }
    else if(c == 0x06) //lcd lines
    {
      if(lcd_lines != 2)
      {
        lcd_lines = 2;
        EEPROM.write(eeprom_addr+3, lcd_lines);
      }
    }
    else if(c == 0x07) //lcd lines
    {
      if(lcd_lines != 1)
      {
        lcd_lines = 1;
        EEPROM.write(eeprom_addr+3, lcd_lines);
      }
    }
    else if(c == 0x08) //re-init lcd
    {
      lcd.begin(lcd_width, lcd_lines);
    }
    else if(c == 0x09) //splash screen on/off
    {
      splash_screen = 1-splash_screen;
      EEPROM.write(eeprom_addr+1, splash_screen);
    }
    else if(c == 0x0A) //alternate splash screen from line arrays (first two lines)
    {
      if(cursor_pos || line_pos)
      {
        ptr = (uint8_t*)&splash_line[0][0];
        for(uint16_t i=0, addr=eeprom_addr+11; i < sizeof(splash_line); i++)
        {
          EEPROM.write(addr++, *ptr++);
        }
      }
      else
      {
        EEPROM.write(eeprom_addr+11, 0x00); //disable alternate splash screen
      }
    }
    else if((c >= 0x0B) && (c <= 0x13))  //baudrate = 2400,4800,9600,14400,19200,38400,57600,115200,28800
    {
      set_baudrate(c-0x0B);
    }
    else if(c == 0x15) //contrast 0-100
    {
      while(Serial.available() == 0);
      c = Serial.read();
      if(c > 97) { contrast = 32;  }
      else       { contrast = c/3; }
      EEPROM.write(eeprom_addr+5, contrast);
    }
    else if(c == 0x16) //rgb backlight on/off
    {
      bl_rgb = 1-bl_rgb;
      if(bl_rgb == 0)
      {
        bl_red   = 100;
        bl_green = 0;
        bl_blue  = 0;
      }
      else
      {
        bl_red   = 100;
        bl_green = 100;
        bl_blue  = 100;
      }
      EEPROM.write(eeprom_addr+10, bl_rgb);
    }
    else if(c == 0x17) //backlight 0-100
    {
      if(bl_rgb == 0)
      {
        while(Serial.available() == 0);
        c = Serial.read();
        bl_red = (c>100)?100:c;
        EEPROM.write(eeprom_addr+7, bl_red);
      }
      else
      {
        while(Serial.available() == 0);
        c = Serial.read();
        bl_red = (c>100)?100:c;
        EEPROM.write(eeprom_addr+7, bl_red);
        while(Serial.available() == 0);
        c = Serial.read();
        bl_green = (c>100)?100:c;
        EEPROM.write(eeprom_addr+8, bl_green);
        while(Serial.available() == 0);
        c = Serial.read();
        bl_blue = (c>100)?100:c;
        EEPROM.write(eeprom_addr+9, bl_blue);
        while(Serial.available() == 0);
      }
      if((bl_on == 0) && (bl_red || bl_green || bl_blue))
      {
        set_backlight(1);
      }
    }
    else if((c > 0x80) &&  (c < 0xA0))//backlight 0-29
    {
      set_backlight(1);
      c = c & 0x1F;
      if(c > 10){ c++; }
      if(c > 20){ c++; }
      if(c > 30){ c++; }
      bl_red = (c>33)?100:(c*3)+1; //129-159 (5-bit)
      EEPROM.write(eeprom_addr+7, bl_red);
    }
  }
  else if(c == 0x08) //backspace
  {
    if((cursor_pos == 0) && (line_pos > 0))
    {
      cursor_pos = lcd_width;
      line_pos = line_pos-1;
      lcd.setCursor(cursor_pos, line_pos);
    }
    if(cursor_pos > 0)
    {
      lcd.command(0x10); //move cursor left
      lcd.write(' ');
      lcd.command(0x10); //move cursor left
      cursor_pos--;
      if((line_pos < 2) && (cursor_pos < 20)) //save data for splash screen
      {
        splash_line[line_pos][cursor_pos] = 0;
      }
    }
  }
  else if((c == 0x0D) || (c == 0x0A)) //CR or LF
  {
    cursor_pos = 0;
    if(++line_pos >= lcd_lines)
    {
      line_pos = 0;
    }
    lcd.setCursor(cursor_pos, line_pos);
  }
  else //print to lcd
  {
    lcd.write(c);
    if((line_pos < 2) && (cursor_pos < 20)) //save data for splash screen
    {
      splash_line[line_pos][cursor_pos]   = c;
      splash_line[line_pos][cursor_pos+1] = 0;
    }
    if(++cursor_pos == lcd_width)
    {
      cursor_pos = 0;
      if(++line_pos >= lcd_lines)
      {
        line_pos = 0;
      }
      lcd.setCursor(cursor_pos, line_pos);
    }
  }
}
