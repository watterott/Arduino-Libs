#ifndef GraphicsLib_h
#define GraphicsLib_h


#include <inttypes.h>
#if defined(__AVR__)
# include <avr/pgmspace.h>
#endif
#include "Print.h"


#ifndef RGB
#define RGB(r,g,b)   (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //RGB565: 5 red | 6 green | 5 blue
#endif

#ifndef SWAP
#define SWAP(x,y)    do{ (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y); }while(0)
#endif

#ifndef DEC
# define DEC (10)
#endif
#ifndef HEX
# define HEX (16)
#endif
#ifndef OCT
# define OCT (8)
#endif
#ifndef BIN
# define BIN (2)
#endif


typedef struct
{
  uint_least32_t x;
  uint_least32_t y;
} CAL_POINT; //calibration points for touchpanel


typedef struct
{
  uint_least32_t a;
  uint_least32_t b;
  uint_least32_t c;
  uint_least32_t d;
  uint_least32_t e;
  uint_least32_t f;
  uint_least32_t div;
} CAL_MATRIX; //calibration matrix for touchpanel


class GraphicsLib : public Print
{
  public:
    uint_least16_t lcd_orientation;      //display orientation/rotation
    int_least16_t lcd_width, lcd_height; //screen size
    int_least16_t lcd_x, lcd_y, lcd_z;   //calibrated pos (screen)
    int_least16_t tp_x, tp_y;            //raw pos (touch panel)
    int_least16_t tp_last_x, tp_last_y;  //last raw pos (touch panel)
    CAL_MATRIX tp_matrix;                //calibration matrix

    GraphicsLib();
    GraphicsLib(uint_least16_t w, uint_least16_t h);

    int_least16_t getWidth(void);
    int_least16_t getHeight(void);
    int_least16_t width(void);
    int_least16_t height(void);
    void setRotation(uint_least16_t r);
    uint_least16_t getRotation(void);

    virtual void invertDisplay(uint_least8_t invert);
    virtual void setOrientation(uint_least16_t o);
    virtual uint_least16_t getOrientation(void);
    virtual void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    virtual void drawStart(void);
    virtual void draw(uint_least16_t color);
    virtual void drawStop(void);
    virtual void fillScreen(uint_least16_t color);
    virtual void drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color);
    virtual void drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color);
    virtual void drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color);
    virtual void fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color);
    virtual void drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color);
    virtual void fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color);
    virtual void drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color);
    virtual void fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color);
    virtual void drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color);
    virtual void fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color);
    virtual void drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color);
    virtual void fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color);
    virtual int_least16_t drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    virtual int_least16_t drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    virtual int_least16_t drawText(int_least16_t x, int_least16_t y, char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    virtual int_least16_t drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
#if defined(__AVR__)
    virtual int_least16_t drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
#endif
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, char val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, unsigned char val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, int val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, unsigned int val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, long val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawInteger(int_least16_t x, int_least16_t y, unsigned long val, uint_least8_t base, uint_least16_t color, uint_least16_t bg, uint_least8_t size);

    //print functions
    void setCursor(int_least16_t x, int_least16_t y);
    void setTextColor(uint_least16_t color);
    void setTextColor(uint_least16_t color, uint_least16_t bg);
    void setTextSize(uint_least8_t size);
    void setTextWrap(uint_least8_t wrap);
#if ARDUINO >= 100
    virtual size_t write(uint8_t c);
#else
    virtual void write(uint8_t c);
#endif

    //touch panel funcions
    virtual uint_least8_t touchRead(void);
    virtual void touchStartCal(void);
    uint_least8_t touchSetCal(CAL_POINT *lcd, CAL_POINT *tp);
    void touchCal(void);
    virtual int_least16_t touchX(void);
    virtual int_least16_t touchY(void);
    virtual int_least16_t touchZ(void);

  private:
    uint_least8_t text_size, text_wrap;
    uint_least16_t text_fg, text_bg;
    int_least16_t text_x, text_y, start_x;
};


#endif //GraphicsLib_h
