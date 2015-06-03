#ifndef MI0283QT9_h
#define MI0283QT9_h


#include <inttypes.h>
#include "GraphicsLib.h"


class MI0283QT9 : public GraphicsLib
{
  public:
    MI0283QT9();

    void begin(uint_least8_t clock_div);
    void begin(void);
    void led(uint_least8_t power);
    void invertDisplay(uint_least8_t invert);
    void setOrientation(uint_least16_t o);
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);

    uint_least8_t touchRead(void);
    void touchStartCal(void);

  private:
    uint_least8_t lcd_clock_div;

    void reset(uint_least8_t clock_div);
    void wr_cmd(uint_least8_t cmd);
    void wr_data16(uint_least16_t data);
    void wr_data(uint_least8_t data);
    uint_least8_t rd_spi(void);
    void wr_spi(uint_least8_t data);
};


#endif //MI0283QT9_h
