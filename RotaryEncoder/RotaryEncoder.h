#ifndef RotaryEncoder_h
#define RotaryEncoder_h


#include <inttypes.h>


#define SW_PRESSED      1
#define SW_PRESSEDLONG  2


class RotaryEncoder
{
  public:
    RotaryEncoder();

    void init(void);
    void service(void);
    int_least8_t step(void);
    int_least8_t sw(void);

  private:
    volatile int_least8_t re_sw;
    volatile int_least8_t re_delta;
    int_least8_t re_last;
};


#endif //RotaryEncoder_h
