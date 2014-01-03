#include <inttypes.h>
#if defined(__AVR__)
# include <avr/pgmspace.h>
#else
# define pgm_read_byte(addr)  (*(const uint8_t *)(addr))
# define pgm_read_word(addr)  (*(const uint16_t *)(addr))
# define pgm_read_dword(addr) (*(const uint32_t *)(addr))
#endif
#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif
#include "fonts.h"
#include "GraphicsLib.h"


//-------------------- Constructor --------------------


GraphicsLib::GraphicsLib(void)
{
  lcd_orientation = 0;
  lcd_width = lcd_height = 128;
  text_size = 1;
  text_wrap = 1;
  text_fg = 0x0000;
  text_bg = 0xFFFF;
  text_x = text_y = 0;

  return;
}


GraphicsLib::GraphicsLib(uint_least16_t w, uint_least16_t h)
{
  lcd_orientation = 0;
  lcd_width = w;
  lcd_height = h;
  text_size = 1;
  text_wrap = 1;
  text_fg = 0x0000;
  text_bg = 0xFFFF;
  text_x = text_y = 0;

  return;
}


//-------------------- Public --------------------


int_least16_t GraphicsLib::getWidth(void)
{
  return lcd_width;
}


int_least16_t GraphicsLib::getHeight(void)
{
  return lcd_height;
}


int_least16_t GraphicsLib::width(void)
{
  return getWidth();
}


int_least16_t GraphicsLib::height(void)
{
  return getHeight();
}


void GraphicsLib::setRotation(uint_least16_t r)
{
  return setOrientation(r);
}


uint_least16_t GraphicsLib::getRotation(void)
{
  return getOrientation();
}


void GraphicsLib::setOrientation(uint_least16_t o)
{
  switch(o)
  {
    default:
    case   0:
      lcd_orientation =   0;
      break;
    case   9:
    case  90:
      lcd_orientation =  90;
      break;
    case  18:
    case 180:
      lcd_orientation = 180;
      break;
    case  27:
    case  14: //270&0xFF
    case 270:
      lcd_orientation = 270;
      break;
  }

  return;
}


uint_least16_t GraphicsLib::getOrientation(void)
{
  return lcd_orientation;
}


void GraphicsLib::invertDisplay(uint_least8_t invert)
{
  return;
}


void GraphicsLib::setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1)
{
  return;
}


void GraphicsLib::drawStart(void)
{
  return;
}


void GraphicsLib::draw(uint_least16_t color)
{
  return;
}


void GraphicsLib::drawStop(void)
{
  return;
}


void GraphicsLib::fillScreen(uint_least16_t color)
{
  uint_least32_t size;

  setArea(0, 0, lcd_width-1, lcd_height-1);

  drawStart();
  for(size=(uint_least32_t)(lcd_width*lcd_height); size!=0; size--)
  {
    draw(color);
  }
  drawStop();

  return;
}


void GraphicsLib::drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color)
{
  if((x0 >= lcd_width) ||
     (y0 >= lcd_height))
  {
    return;
  }

  setArea(x0, y0, x0, y0);

  drawStart();
  draw(color);
  drawStop();

  return;
}


void GraphicsLib::drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color)
{
	int_least16_t dx, dy, dx2, dy2, err, stepx, stepy;

  if(x0 == x1) //horizontal line
  {
    if(y0 > y1){ SWAP(y1, y0); }
    fillRect(x0, y0, 1, y1-y0+1, color);
  }
  else if(y0 == y1) //vertical line
  {
    if(x0 > x1){ SWAP(x1, x0); }
    fillRect(x0, y0, x1-x0+1, 1, color);
  }
  else
  {
    //check parameter
    if(x0 >= lcd_width)  { x0 = lcd_width-1;  }
    if(y0 >= lcd_height) { y0 = lcd_height-1; }
    if(x1 >= lcd_width)  { x1 = lcd_width-1;  }
    if(y1 >= lcd_height) { y1 = lcd_height-1; }
    //calculate direction
    dx = x1 - x0;
    dy = y1 - y0;
    if(dx < 0) { dx = -dx; stepx = -1; } else { stepx = +1; }
    if(dy < 0) { dy = -dy; stepy = -1; } else { stepy = +1; }
    dx2 = dx << 1;
    dy2 = dy << 1;
    //draw line
    drawPixel(x0, y0, color);
    if(dx > dy)
    {
      err = dy2 - dx;
      while(x0 != x1)
      {
        if(err >= 0)
        {
          y0  += stepy;
          err -= dx2;
        }
        x0  += stepx;
        err += dy2;
        drawPixel(x0, y0, color);
      }
    }
    else
    {
      err = dx2 - dy;
      while(y0 != y1)
      {
        if(err >= 0)
        {
          x0  += stepx;
          err -= dy2;
        }
        y0  += stepy;
        err += dx2;
        drawPixel(x0, y0, color);
      }
    }
  }

  return;
}


void GraphicsLib::drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  fillRect(x0,     y0,     1, h, color);
  fillRect(x0,     y0+h-1, w, 1, color);
  fillRect(x0+w-1, y0,     1, h, color);
  fillRect(x0,     y0,     w, 1, color);

  return;
}


void GraphicsLib::fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color)
{
  uint_least32_t size;

  if(x0   >= lcd_width)  { x0 = lcd_width-1;  }
  if(y0   >= lcd_height) { y0 = lcd_height-1; }
  if(x0+w >= lcd_width)  { w  = lcd_width-1-x0;  }
  if(y0+h >= lcd_height) { h  = lcd_height-1-y0; }

  setArea(x0, y0, x0+w-1, y0+h-1);

  drawStart();

  for(size=(w*h); size!=0; size--)
  {
    draw(color);
  }

  drawStop();

  return;
}



void GraphicsLib::drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);

  return;
}


void GraphicsLib::fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color)
{
  int_least16_t a, b, y, last, sa, sb;
  int_least16_t dx01, dy01, dx02, dy02, dx12, dy12;

  //sort coordinates (y2 >= y1 >= y0)
  if(y0 > y1){ SWAP(y0, y1); SWAP(x0, x1); }
  if(y1 > y2){ SWAP(y2, y1); SWAP(x2, x1); }
  if(y0 > y1){ SWAP(y0, y1); SWAP(x0, x1); }
  
  if(y0 == y2) //all on the same line
  { 
    a = b = x0;
         if(x1 < a){ a = x1; }
    else if(x1 > b){ b = x1; }
         if(x2 < a){ a = x2; }
    else if(x2 > b){ b = x2; }
    drawLine(a, y0, b, y0, color);
    return;
  }

  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1;

  //upper part of triangle
  if(y1 == y2) last = y1;
  else         last = y1-1;

  for(y=y0, sa=0, sb=0; y<=last; y++)
  {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    drawLine(a, y, b, y, color);
  }

  //lower part of triangle
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++)
  {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    drawLine(a, y, b, y, color);
  }

  return;
}


void GraphicsLib::drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  int_least16_t err, x, y, x1=x0+w-1, y1=y0+h-1;

  //draw horizontal + vertical lines
  fillRect(x0,   y0+r, 1,   h-r-r, color);
  fillRect(x0+r, y1,   w-r-r, 1,   color);
  fillRect(x1,   y0+r, 1,   h-r-r, color);
  fillRect(x0+r, y0,   w-r-r, 1,   color);

  //calculate corner positions
  y1 = y1-r;
  x1 = x1-r;
  x0 = x0+r;
  y0 = y0+r;

  //draw round corners
  err = -r;
  x   = r;
  y   = 0;
  while(x >= y)
  {
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 - y, y0 - x, color);
    drawPixel(x1 + x, y0 - y, color);
    drawPixel(x1 + y, y0 - x, color);
    drawPixel(x1 + x, y1 + y, color);
    drawPixel(x1 + y, y1 + x, color);
    drawPixel(x0 - x, y1 + y, color);
    drawPixel(x0 - y, y1 + x, color);

    err += y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err = err - x - x;
    }
  }

  return;
}


void GraphicsLib::fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color)
{
  int_least16_t err, x, y, x1=x0+w, y1=y0+h;

  fillRect(x0+r+1, y0,   w-r-r-2, h,     color);
  fillRect(x0,     y0+r, r+1,     h-r-r, color);
  fillRect(x1-r-1, y0+r, r+1,     h-r-r, color);

  //calculate corner positions
  y1 = y1-r-1;
  x1 = x1-r-1;
  x0 = x0+r;
  y0 = y0+r;

  //fill round corners
  err = -r;
  x   = r;
  y   = 0;
  while(x >= y)
  {
    drawLine(x0 - x, y0 - y, x0, y0 - y, color);
    drawLine(x0 - y, y0 - x, x0, y0 - x, color);
    drawLine(x1 + x, y0 - y, x1, y0 - y, color);
    drawLine(x1 + y, y0 - x, x1, y0 - x, color);
    drawLine(x1 + x, y1 + y, x1, y1 + y, color);
    drawLine(x1 + y, y1 + x, x1, y1 + x, color);
    drawLine(x0 - x, y1 + y, x0, y1 + y, color);
    drawLine(x0 - y, y1 + x, x0, y1 + x, color);

    err += y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err = err - x - x;
    }
  }

  return;
}


void GraphicsLib::drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t radius, uint_least16_t color)
{
  int_least16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  while(x >= y)
  {
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);

    err += y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err = err - x - x;
    }
  }

  return;
}


void GraphicsLib::fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t radius, uint_least16_t color)
{
  int_least16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  while(x >= y)
  {
    drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);

    err += y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err = err - x - x;
    }
  }

  return;
}


void GraphicsLib::drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  int_least16_t x, y;
  int_least32_t e, e2, dx, dy, rx, ry;

  x  = -r_x;
  y  = 0;
  dx = (1+(2*x))*r_y*r_y;
  dy = x*x;
  e  = dx+dy;
  rx = 2*r_x*r_x;
  ry = 2*r_y*r_y;

  while(x <= 0)
  {
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);

    e2 = 2*e;
    if(e2 >= dx) { x++; dx += ry; e += dx; }
    if(e2 <= dy) { y++; dy += rx; e += dy; }
  }

  while(y++ < r_y)
  {
    drawPixel(x0, y0 + y, color);
    drawPixel(x0, y0 - y, color);
  }

  return;
}


void GraphicsLib::fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color)
{
  int_least16_t x, y;
  int_least32_t e, e2, dx, dy, rx, ry;

  if(((x0-r_x) >= lcd_width) ||
     ((y0-r_y) >= lcd_width))
  {
    return;
  }

  x  = -r_x;
  y  = 0;
  dx = (1+(2*x))*r_y*r_y;
  dy = x*x;
  e  = dx+dy;
  rx = 2*r_x*r_x;
  ry = 2*r_y*r_y;

  while(x <= 0)
  {
    fillRect(x0 + x, y0 + y, 2*-x, 1, color);
    fillRect(x0 + x, y0 - y, 2*-x, 1, color);

    e2 = 2*e;
    if(e2 >= dx) { x++; dx += ry; e += dx; }
    if(e2 <= dy) { y++; dy += rx; e += dy; }
  }

  while(y++ < r_y)
  {
    drawPixel(x0, y0 + y, color);
    drawPixel(x0, y0 - y, color);
  }

  return;
}


int_least16_t GraphicsLib::drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  int_least16_t ret, pos;
#if FONT_WIDTH <= 8
  uint_least8_t data, mask;
#elif FONT_WIDTH <= 16
  uint_least16_t data, mask;
#elif FONT_WIDTH <= 32
  uint_least32_t data, mask;
#endif
  uint_least8_t i, j, width, height;
#if defined(__AVR__)
  const prog_uint8_t *ptr;
#else
  uint8_t *ptr;
#endif

#if FONT_WIDTH <= 8
  pos = (c-FONT_START)*(8*FONT_HEIGHT/8);
#elif FONT_WIDTH <= 16
  pos = (c-FONT_START)*(16*FONT_HEIGHT/8);
#elif FONT_WIDTH <= 32
  pos = (c-FONT_START)*(32*FONT_HEIGHT/8);
#endif
  ptr    = &font_PGM[pos];
  width  = FONT_WIDTH;
  height = FONT_HEIGHT;
  size   = size&0x7F;

  if(size <= 1)
  {
    ret = x+width;
    if((ret-1) >= lcd_width)
    {
      return lcd_width+1;
    }
    else if((y+height-1) >= lcd_height)
    {
      return lcd_width+1;
    }

    setArea(x, y, (x+width-1), (y+height-1));

    drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = pgm_read_dword(ptr); ptr+=4;
#endif
      for(mask=(1<<(width-1)); mask!=0; mask>>=1)
      {
        if(data & mask)
        {
          draw(color);
        }
        else
        {
          draw(bg);
        }
      }
    }
    drawStop();
  }
  else
  {
    ret = x+(width*size);
    if((ret-1) >= lcd_width)
    {
      return lcd_width+1;
    }
    else if((y+(height*size)-1) >= lcd_height)
    {
      return lcd_width+1;
    }

    setArea(x, y, (x+(width*size)-1), (y+(height*size)-1));

    drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = pgm_read_dword(ptr); ptr+=4;
#endif
      for(i=size; i!=0; i--)
      {
        for(mask=(1<<(width-1)); mask!=0; mask>>=1)
        {
          if(data & mask)
          {
            for(j=size; j!=0; j--)
            {
              draw(color);
            }
          }
          else
          {
            for(j=size; j!=0; j--)
            {
              draw(bg);
            }
          }
        }
      }
    }
    drawStop();
  }

  return ret;
}


int_least16_t GraphicsLib::drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  return drawChar(x, y, (char) c, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  while(*s != 0)
  {
    x = drawChar(x, y, (char)*s++, color, bg, size);
    if(x > lcd_width)
    {
      break;
    }
  }

  return x;
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, unsigned char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  return drawText(x, y, (char*) s, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, int i, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16];

  itoa(i, tmp, 10);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, unsigned int i, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16];

  utoa(i, tmp, 10);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, long l, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16];

  ltoa(l, tmp, 10);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, unsigned long l, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16];

  ultoa(l, tmp, 10);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  for(uint_least16_t i=0; i < s.length(); i++) 
  {
    x = drawChar(x, y, (char)s[i], color, bg, size);
    if(x > lcd_width)
    {
      break;
    }
  }

  return x;
}


#if defined(__AVR__)
int_least16_t GraphicsLib::drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char c;

  c = pgm_read_byte(s++);
  while(c != 0)
  {
    x = drawChar(x, y, c, color, bg, size);
    if(x > lcd_width)
    {
      break;
    }
    c = pgm_read_byte(s++);
  }

  return x;
}
#endif


int_least16_t GraphicsLib::drawInteger(int_least16_t x, int_least16_t y, char val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16+1];

  itoa((int)val, tmp, base);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawInteger(int_least16_t x, int_least16_t y, unsigned char val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16+1];

  utoa((int)val, tmp, base);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawInteger(int_least16_t x, int_least16_t y, int val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16+1];

  itoa((int)val, tmp, base);

  return drawText(x, y, tmp, color, bg, size);
}


int_least16_t GraphicsLib::drawInteger(int_least16_t x, int_least16_t y, long val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size)
{
  char tmp[16+1];

  ltoa((int)val, tmp, base);

  return drawText(x, y, tmp, color, bg, size);
}


void GraphicsLib::setCursor(int_least16_t x, int_least16_t y)
{
  if((x < lcd_width) &&
     (y < lcd_height))
  {
    text_x = x;
    text_y = y;
  }

  return;
}


void GraphicsLib::setTextColor(uint_least16_t color)
{
  text_fg = color;

  return;
}


void GraphicsLib::setTextColor(uint_least16_t color, uint_least16_t bg)
{
  text_fg = color;
  text_bg = bg;

  return;
}


void GraphicsLib::setTextSize(uint_least8_t size)
{
  text_size = size;

  return;
}


void GraphicsLib::setTextWrap(uint_least8_t wrap)
{
  text_wrap = (wrap!=0)?1:0;

  return;
}


#if ARDUINO >= 100
size_t GraphicsLib::write(uint8_t c)
#else
void GraphicsLib::write(uint8_t c)
#endif
{
  if(c == '\n')
  {
    text_x  = 0;
    text_y += (text_size&0x7F)*FONT_HEIGHT;
  }
  else if((c != '\r') && (c != '\t'))//skip
  {
    text_x = drawChar(text_x, text_y, c, text_fg, text_bg, text_size);
    if(text_wrap && (text_x > (lcd_width-((text_size&0x7F)*FONT_WIDTH))))
    {
      text_x = 0;
      text_y += (text_size&0x7F)*FONT_HEIGHT;
      if(text_y > (lcd_height-((text_size&0x7F)*FONT_HEIGHT)))
      {
        text_y = 0;
      }
    }
  }

#if ARDUINO >= 100
  return 1;
#else
  return;
#endif
}
