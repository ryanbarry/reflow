#ifndef _TEMP_H
#define _TEMP_H

#include <Arduino.h>
#include "MAX31855.h"

class Thermocouple {
public:
  volatile int32_t temp, internalTemp;
  volatile uint8_t err;

  Thermocouple(void);

  void readAll(void);

private:
  MAX31855 *tc;
};

#endif
