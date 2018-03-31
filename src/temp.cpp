#include "temp.h"

#include "pins.h"

Thermocouple::Thermocouple(void) {
  tc = new MAX31855(PIN_M_CLK, PIN_M_CS, PIN_M_DO);
  tc->begin();

  temp = 0;
  internalTemp = 0;
  err = 0;
}

void Thermocouple::readAll(void) {
  //internalTemp = tc->read16thDegInternal();
  temp = tc->readQtrDegCelsius();
  err = tc->readError();
}
