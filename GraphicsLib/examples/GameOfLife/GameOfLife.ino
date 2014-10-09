/*
  Game Of Life Demonstration
 */

#include <Wire.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <SSD1331.h>
#include <S65L2F50.h>
#include <S65LPH88.h>
#include <S65LS020.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>


//Declare only one display !
// SSD1331 lcd;
// S65L2F50 lcd;
// S65LPH88 lcd;
// S65LS020 lcd;
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)


uint16_t drawcolor[5] = {RGB( 15, 15, 15),  //bg
                         RGB(  0,  0,255),  //alive
                         RGB(  0,  0,  0),  //dead
                         RGB(  0,  0, 40),  //die1
                         RGB(  0,  0,100)}; //die2
#define BG_COLOR    (0)
#define ALIVE_COLOR (1)
#define DEAD_COLOR  (2)
#define DIE1_COLOR  (3)
#define DIE2_COLOR  (4)

#define MAX_GEN     (1000) //max generations
#define X_SIZE      (20)   //cell width
#define Y_SIZE      (15)   //cell height
uint8_t frame[X_SIZE][Y_SIZE];

uint16_t generation=0; //generation counter
#define DEL_CELL    (3)
#define ON_CELL     (0xAA)
#define NEW_CELL    (0xFF)


uint8_t alive(uint8_t x, uint8_t y)
{
  if((x < X_SIZE) && (y < Y_SIZE))
  {
    if((frame[x][y] == ON_CELL) || (frame[x][y] == NEW_CELL))
    {
      return 1;
    }
  }

  return 0;
}


uint8_t neighbors(uint8_t x, uint8_t y)
{
  uint8_t count=0;

  //3 above
  if(alive(x-1,y-1)){ count++; }
  if(alive(x,  y-1)){ count++; }
  if(alive(x+1,y-1)){ count++; }

  //2 on each side
  if(alive(x-1,  y)){ count++; }
  if(alive(x+1,  y)){ count++; }

  //3 below
  if(alive(x-1,y+1)){ count++; }
  if(alive(  x,y+1)){ count++; }
  if(alive(x+1,y+1)){ count++; }

  return count;
}


void play_gol(void)
{
  uint8_t x, y, count;

  //update cells
  for(x=0; x < X_SIZE; x++)
  {
    for(y=0; y < Y_SIZE; y++)
    {
      count = neighbors(x, y);
      if((count == 3) && !alive(x, y))
      {
        frame[x][y] = NEW_CELL; //new cell
      }
      if(((count < 2) || (count > 3)) && alive(x, y))
      {
        frame[x][y] = DEL_CELL; //cell dies
      }
    }
  }

  //increment generation counter
  if(++generation > MAX_GEN)
  {
    init_gol();
  }
}


void draw_gol(void)
{
  uint8_t c, x, y, color;
  uint16_t px, py;

  for(x=0, px=0; x < X_SIZE; x++)
  {
    for(y=0, py=0; y < Y_SIZE; y++)
    {
      c = frame[x][y];
      if(c && (c != ON_CELL))
      {
        if(c == NEW_CELL) //new
        {
          frame[x][y] = ON_CELL;
          color = ALIVE_COLOR;
        }
        else if(c == 1) //del
        {
          frame[x][y] = 0;
          color = DEAD_COLOR;
        }
        else if(c == 2) //die
        {
          frame[x][y] = 1;
          color = DIE1_COLOR;
        }
        else if(c <= DEL_CELL) //die
        {
          frame[x][y]--;
          color = DIE2_COLOR;
        }
        lcd.fillRect(px+1, py+1, (lcd.getWidth()/X_SIZE)-2, (lcd.getHeight()/Y_SIZE)-2, drawcolor[color]);
      }
      py += (lcd.getHeight()/Y_SIZE);
    }
    px += (lcd.getWidth()/X_SIZE);
  }
}


void init_gol(void)
{
  uint8_t x, y;
  uint16_t px, py;
  uint32_t c;

  //reset generation counter
  generation = 0;

  //generate random start data
  srand(frame[X_SIZE/2][Y_SIZE/2] + analogRead(0));
  for(x=0; x < X_SIZE; x++)
  {
    c = (rand() | (rand()<<16)) & 0xAAAAAAAA ; //0xAAAAAAAA 0x33333333 0xA924A924
    for(y=0; y < Y_SIZE; y++)
    {
      if(c & (1<<y))
      {
        frame[x][y] = NEW_CELL;
      }
      else
      {
        frame[x][y] = 0;
      }
    }
  }

  //redraw cells
  for(x=0, px=0; x < X_SIZE; x++)
  {
    for(y=0, py=0; y < Y_SIZE; y++)
    {
      lcd.fillRect(px+1, py+1, (lcd.getWidth()/X_SIZE)-2, (lcd.getHeight()/Y_SIZE)-2, drawcolor[DEAD_COLOR]);
      py += (lcd.getHeight()/Y_SIZE);
    }
    px += (lcd.getWidth()/X_SIZE);
  }
}


void setup()
{
  //init display
  lcd.begin(); //spi-clk=SPI_CLOCK_DIV4
  //lcd.begin(SPI_CLOCK_DIV2); //spi-clk=SPI_CLOCK_DIV2
  //lcd.begin(SPI_CLOCK_DIV4, 8); //SPI Displays: spi-clk=Fcpu/4, rst-pin=8
  //lcd.begin(0x20); //I2C Displays: addr=0x20
  //lcd.begin(0x20, 8); //I2C Displays: addr=0x20, rst-pin=8

  //clear screen
  lcd.fillScreen(drawcolor[BG_COLOR]);

  //init gol
  init_gol();
}


void loop()
{
  unsigned long m;
  static unsigned long prevMillis=0;

  m = millis();
  if((m - prevMillis) >= 65) //update every 65ms (15Hz)
  {
    prevMillis = m;

    if(lcd.touchRead()) //touch press?
    {
      init_gol();
    }
    else
    {
      play_gol();
      draw_gol();
    }

    //show current generation
    lcd.drawInteger(1, 1, generation, DEC, drawcolor[DIE2_COLOR], drawcolor[DEAD_COLOR], 1);
  }
}
