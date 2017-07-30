#ifndef S65LS020_h
#define S65LS020_h


#include <inttypes.h>
#include "GraphicsLib.h"


class S65LS020 : public GraphicsLib
{
  public:
    S65LS020();

    void begin(uint_least8_t clock_div);
    void begin(void);
    void led(uint_least8_t power);
    void invertDisplay(uint_least8_t invert);
    void setOrientation(uint_least16_t o);
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);

  private:
    void reset(void);
    void wr_cmd(uint_least8_t cmd, uint_least8_t param);
    void wr_spi(uint_least8_t data);
};


#endif //S65LS020_h
