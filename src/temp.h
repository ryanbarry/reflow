#ifndef _TEMP_H
#define _TEMP_H

#include <Arduino.h>
#include "MAX31855.h"

//MAX38155 thermocouple interface
#define M_CLK 13 //SCK0
#define M_CS 6
#define M_DO 12  //MISO0

extern volatile int32_t temp, tcint;
extern volatile uint8_t tcerr;

extern MAX31855 tc;

#endif
