#include "temp.h"
#include "pins.h"

Thermocouple::Thermocouple(void) {
  max = new MAX31855(PIN_M_CLK, PIN_M_CS, PIN_M_DO);
  max->begin();

  avgtmp = 0;
  tcReadingCounter = 0;
}

void Thermocouple::tcReadIsr(void) {
  static int readingNum = 0;
  static int32_t readingHistory[AVG_READINGS];
  static uint8_t errorCounter = 0;

  int32_t reading = max->readQtrDegCelsius();
  err = max->readError();

  if (err == MAX31855_NO_ERR) {
    readingHistory[tcReadingCounter % AVG_READINGS] = reading;

    int32_t sum = 0;

    for(int i=0; i< AVG_READINGS; i++)
      sum += readingHistory[i];

    avgtmp = sum/AVG_READINGS;

    tcReadingCounter++;
    errorCounter = 0;
  } else {
    errorCounter++;
    if (errorCounter > ERR_THRESHOLD) {
      avgtmp = -errorCounter;
    }
  }
}

