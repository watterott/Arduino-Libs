#ifndef MI0283QT2_h
#define MI0283QT2_h


#include <inttypes.h>
#include "../GraphicsLib/GraphicsLib.h"


class MI0283QT2 : public GraphicsLib
{
  public:
    MI0283QT2();

    void begin(uint_least8_t clock_div, uint_least8_t rst_pin);
    void begin(uint_least8_t clock_div); //SPI_CLOCK_DIV2 SPI_CLOCK_DIV4 SPI_CLOCK_DIV8 SPI_CLOCK_DIV16 SPI_CLOCK_DIV32 SPI_CLOCK_DIV64 SPI_CLOCK_DIV128
    void led(uint_least8_t power); //0-100
    void setOrientation(uint_least16_t o); //0 90 180 270
    void setArea(int_least16_t x0, int_least16_t y0, int_least16_t x1, int_least16_t y1);
    void fillScreen(uint_least16_t color);
    void drawStart(void);
    void draw(uint_least16_t color);
    void drawStop(void);

  private:
    void reset(uint_least8_t clock_div, uint_least8_t rst_pin);
    void wr_cmd(uint_least8_t cmd, uint_least8_t param);
    void wr_spi(uint_least8_t data);
};


#endif //MI0283QT2_h
