#ifndef _PINS_H
#define _PINS_H

#include <stdint.h>

//MAX38155 thermocouple interface
#define M_CLK 13 //SCK0
#define M_CS 6
#define M_DO 12  //MISO0

//LCD controller
#define LIGHT_GREY ((0x8<<11)|(0x10<<5)|0x8)
#define TFT_DC 9
#define TFT_CS 10

//Touch controller
#define T_CS  8
#define T_IRQ 4

//outputs
#define NUM_OUTPUTS 5
#define NUM_HEATERS 3
#define OUTPUT_PINS_HEATER_TOP 0
#define OUTPUT_PINS_HEATER_BOTTOM 1
#define OUTPUT_PINS_HEATER_BOOST 2
#define OUTPUT_PINS_FAN_COOLING 3
#define OUTPUT_PINS_SERVO_DOOR 4
extern uint8_t output_pins[5];

#define OUTPUT_ON(p) digitalWrite(output_pins[p], 1)
#define OUTPUT_OFF(p) digitalWrite(output_pins[p], 0)
#define ALL_HEATERS_OFF OUTPUT_OFF(OUTPUT_PINS_HEATER_TOP); OUTPUT_OFF(OUTPUT_PINS_HEATER_BOTTOM); OUTPUT_OFF(OUTPUT_PINS_HEATER_BOOST);

#endif
