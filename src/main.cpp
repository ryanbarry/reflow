#include <Arduino.h>
#include "MAX31855.h"

//MAX38155 thermocouple interface
#define M_CLK 13 //SCK0
#define M_CS 6
#define M_DO 12  //MISO0

//Touch controller
#define T_CS  8
#define T_IRQ 4

#include "display.h"

MAX31855 tc(M_CLK, M_CS, M_DO);
volatile int32_t temp, tcint;
volatile uint8_t tcerr;

IntervalTimer tcTimer;

void readTcIsr() {
  tcint = tc.read16thDegInternal();
  temp = tc.readQtrDegCelsius();
  tcerr = tc.readError();
}

void setup() {
  Serial.begin(0);
  setupDisplay();
  tc.begin();

  //while (!Serial.dtr());
  clearDisplay();
  drawHeader("Reflow");

  tcTimer.begin(readTcIsr, 200000);
}

elapsedMillis m;
uint32_t numS = 0;

void loop() {
  if (m/1000 > numS) {
    numS++;
    displayTemp();
  }
}
