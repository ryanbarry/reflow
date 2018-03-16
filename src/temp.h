#ifndef _TEMP_H
#define _TEMP_H

#include <Arduino.h>
#include "MAX31855.h"
#include "pins.h"

extern volatile int32_t temp, tcint;
extern volatile uint8_t tcerr;

extern MAX31855 tc;

#endif
