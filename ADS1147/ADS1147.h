#ifndef ADS1147_h
#define ADS1147_h


#include <inttypes.h>


class ADS1147
{
  public:
    ADS1147();
    uint_least8_t begin(uint_least8_t drate, uint_least8_t gain, uint_least16_t current);
    uint_least8_t begin(void);
    int_least16_t read(uint_least8_t chn);

  private:
    int_least16_t rd_data(void);
    void wr_cmd(uint_least8_t cmd);
    uint_least8_t rd_reg(uint_least8_t reg);
    void wr_reg(uint_least8_t reg, uint_least8_t data);
    uint_least8_t rd_spi(void);
    void wr_spi(uint_least8_t data);
};


#endif //ADS1147_h
