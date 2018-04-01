#ifndef _PINS_H
#define _PINS_H

#include <stdint.h>

//MAX38155 thermocouple interface
#define PIN_M_CLK 13 //SCK0
#define PIN_M_CS 6
#define PIN_M_DO 12  //MISO0

//LCD controller
#define LIGHT_GREY ((0x8<<11)|(0x10<<5)|0x8)
#define PIN_TFT_DC 9
#define PIN_TFT_CS 10

//Touch controller
#define PIN_T_CS  8
#define PIN_T_IRQ 4

//piezo
#define PIN_BUZZER 22

//outputs
#define NUM_OUTPUTS 5
#define NUM_HEATERS 3
#define PIN_HEATER_TOP 1
#define PIN_HEATER_BOTTOM 0
#define PIN_HEATER_BOOST 2
#define PIN_FAN_COOLING 16
#define PIN_SERVO_DOOR 17
extern uint8_t output_pins[];

#define OUTPUT_ON(p) digitalWrite(p, 1)
#define OUTPUT_OFF(p) digitalWrite(p, 0)
#define ALL_HEATERS_OFF OUTPUT_OFF(PIN_HEATER_TOP); OUTPUT_OFF(PIN_HEATER_BOTTOM); OUTPUT_OFF(PIN_HEATER_BOOST);

void setupPins(void);

#endif
