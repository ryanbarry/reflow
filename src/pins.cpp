#include "pins.h"
#include <Arduino.h>

uint8_t output_pins[] = {PIN_HEATER_TOP, PIN_HEATER_BOTTOM, PIN_HEATER_BOOST, PIN_FAN_COOLING, PIN_SERVO_DOOR};

void setupPins() {
  for(int i=0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
#ifdef DEBUG
    Serial.print("setting pin "); Serial.print(output_pins[i]); Serial.println(" OUTPUT");
#endif
    pinMode(i, OUTPUT);
  }
}


