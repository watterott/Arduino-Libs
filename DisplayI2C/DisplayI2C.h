#ifndef DisplayI2C_h
#define DisplayI2C_h


#include <inttypes.h>
#if defined(__AVR__)
# include <avr/pgmspace.h>
#endif
#include "GraphicsLib.h"


class DisplayI2C : public GraphicsLib
{
  public:
    DisplayI2C();

    void begin(uint_least8_t addr, uint_least8_t rst_pin);
    void begin(uint_least8_t addr);
    void begin(void);
    uint_least8_t getSize(void);
    uint_least8_t getVersion(char *v);
    uint_least8_t getFeatures(void);
    void setFeatures(uint_least8_t f);
    void led(uint_least8_t power);
    void invertDisplay(uint_least8_t invert);
    void setOrientation(uint_least16_t o);
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);
    void fillScreen(uint_least16_t color);
    void drawPixel(int_least16_t x0, int_least16_t y0, uint_least16_t color);
    void drawLine(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, uint_least16_t color);
    void drawRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color);
    void fillRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, uint_least16_t color);
    void drawTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color);
    void fillTriangle(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1, int_least16_t x2, int_least16_t y2, uint_least16_t color);
    void drawRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color);
    void fillRoundRect(int_least16_t x0, int_least16_t y0, int_least16_t w, int_least16_t h, int_least16_t r, uint_least16_t color);
    void drawCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color);
    void fillCircle(int_least16_t x0, int_least16_t y0, int_least16_t r, uint_least16_t color);
    void drawEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color);
    void fillEllipse(int_least16_t x0, int_least16_t y0, int_least16_t r_x, int_least16_t r_y, uint_least16_t color);
    int_least16_t drawChar(int_least16_t x, int_least16_t y, char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawChar(int_least16_t x, int_least16_t y, unsigned char c, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawText(int_least16_t x, int_least16_t y, char *s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
    int_least16_t drawText(int_least16_t x, int_least16_t y, String &s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
#if defined(__AVR__)
    int_least16_t drawTextPGM(int_least16_t x, int_least16_t y, PGM_P s, uint_least16_t color, uint_least16_t bg, uint_least8_t size);
#endif

    //touch panel funcions
    uint_least8_t touchRead(void);
    void touchStartCal(void);
    int_least16_t touchX(void);
    int_least16_t touchY(void);
    int_least16_t touchZ(void);

  private:
    uint_least8_t bigger_8bit, features;
    int_least16_t display_width, display_height;
    int i2c_addr;
};


#endif //DisplayI2C_h
