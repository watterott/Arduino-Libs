#ifndef ADS7846_h
#define ADS7846_h


#include <inttypes.h>
#include "../GraphicsLib/GraphicsLib.h"
#include "../MI0283QT2/MI0283QT2.h"
#include "../MI0283QT9/MI0283QT9.h"


typedef struct 
{
  uint_least16_t x;
  uint_least16_t y;
} TP_POINT;


typedef struct 
{
  uint_least32_t x;
  uint_least32_t y;
} CAL_POINT;


typedef struct 
{
  uint_least32_t a;
  uint_least32_t b;
  uint_least32_t c;
  uint_least32_t d;
  uint_least32_t e;
  uint_least32_t f;
  uint_least32_t div;
} CAL_MATRIX;


class ADS7846
{
  public:
    uint_least16_t lcd_orientation; //lcd_orientation
    TP_POINT lcd;                   //calibrated pos (screen)
    TP_POINT tp;                    //raw pos (touch panel)
    TP_POINT tp_last;               //last raw pos (touch panel)
    CAL_MATRIX tp_matrix;           //calibrate matrix
    uint_least8_t pressure;         //touch panel pressure

    ADS7846();

    void begin(void);
    void setOrientation(uint_least16_t o);
    void setRotation(uint_least16_t r);
    uint_least8_t setCalibration(CAL_POINT *lcd, CAL_POINT *tp);
    uint_least8_t writeCalibration(uint16_t eeprom_addr);
    uint_least8_t readCalibration(uint16_t eeprom_addr);
    uint_least8_t doCalibration(MI0283QT2 *lcd, uint16_t eeprom_addr, uint_least8_t check_eeprom);
    uint_least8_t doCalibration(MI0283QT9 *lcd, uint16_t eeprom_addr, uint_least8_t check_eeprom);
    void calibrate(void);
    uint_least16_t getX(void);
    uint_least16_t getY(void);
    uint_least16_t getXraw(void);
    uint_least16_t getYraw(void);
    uint_least8_t getPressure(void);
    void service(void);

  private:
    void rd_data(void);
    uint8_t rd_spi(void);
    void wr_spi(uint_least8_t data);
};


#endif //ADS7846_h
