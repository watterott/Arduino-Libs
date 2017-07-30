#ifndef SSD1331_h
#define SSD1331_h


#include <inttypes.h>
#include "GraphicsLib.h"


class SSD1331 : public GraphicsLib
{
  public:
    SSD1331();

    void begin(uint_least8_t clock_div);
    void begin(void);
    void invertDisplay(uint_least8_t invert);
    void setOrientation(uint_least16_t o);
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);

  private:
    void reset(void);
    void wr_cmd(uint_least8_t cmd);
    void wr_data(uint_least8_t data);
    void wr_spi(uint_least8_t data);
};


#endif //SSD1331_h
