#ifndef _TEMP_H
#define _TEMP_H

#include <Arduino.h>
#include <stdint.h>
#include "MAX31855.h"

#define ERR_THRESHOLD 5
#define AVG_READINGS 15

class Thermocouple {
public:
  volatile uint32_t tcReadingCounter;
  volatile int32_t avgtmp;
  volatile uint8_t err;

  Thermocouple(void);

  void tcReadIsr(void);

private:
  MAX31855 *max;
};

#endif
