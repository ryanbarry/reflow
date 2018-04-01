#include "temp.h"
#include "pins.h"

Thermocouple tc;
volatile uint32_t tcReadingCounter = 0;

Thermocouple::Thermocouple(void) {
  max = new MAX31855(PIN_M_CLK, PIN_M_CS, PIN_M_DO);
  max->begin();

  temp = 0;
  internalTemp = 0;
  err = 0;
}

void Thermocouple::readAll(void) {
  //internalTemp = tc->read16thDegInternal();
  temp = max->readQtrDegCelsius();
  err = max->readError();
}

void tcReadIsr(void) {
  tc.readAll();
  tcReadingCounter++;
}

