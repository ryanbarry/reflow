#include "temp.h"

volatile int32_t temp = 0, tcint = 0;
volatile uint8_t tcerr = 0;

MAX31855 tc(M_CLK, M_CS, M_DO);
