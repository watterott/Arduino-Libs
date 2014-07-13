#ifndef DAC8760_h
#define DAC8760_h


#include <inttypes.h>


class DAC8760
{
  public:
    DAC8760();
    void begin(void);
    void write(uint_least16_t val);

  private:
    void wr_reg(uint_least8_t reg, uint_least16_t data);
    void wr_spi(uint_least8_t data);
};


#endif //DAC8760_h
