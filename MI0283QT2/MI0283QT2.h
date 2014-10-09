#ifndef MI0283QT2_h
#define MI0283QT2_h


#include <inttypes.h>
#include "GraphicsLib.h"


class MI0283QT2 : public GraphicsLib
{
  public:
    MI0283QT2();

    void begin(uint_least8_t clock_div);
    void begin(void);
    void led(uint_least8_t power);
    void setOrientation(uint_least16_t o);
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);

    uint_least8_t touchRead(void);
    void touchStartCal(void);

  private:
    void reset(uint_least8_t clock_div);
    void wr_cmd(uint_least8_t cmd, uint_least8_t param);
    uint_least8_t rd_spi(void);
    void wr_spi(uint_least8_t data);
};


#endif //MI0283QT2_h
