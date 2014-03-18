#ifndef S65L2F50_h
#define S65L2F50_h


#include <inttypes.h>
#include "../GraphicsLib/GraphicsLib.h"


class S65L2F50 : public GraphicsLib
{
  public:
    S65L2F50();

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
    void reset(uint_least8_t clock_div);
    void wr_cmd(uint_least8_t cmd);
    void wr_dat(uint_least8_t data);
    void wr_spi(uint_least8_t data);
};


#endif //S65L2F50_h
