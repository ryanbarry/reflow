#include <Arduino.h>

#include "temp.h"
#include "learn.h"
#include "bake.h"
#include "reflow.h"
#include "display.h"

#define THERMOCOUPLE_READ_INTERVAL_MILLIS 200

IntervalTimer tcTimer;
Thermocouple tc;

void tcRead(void) {
  tc.readAll();
}

void setup() {
  Serial.begin(0);
  setupDisplay();

  //while (!Serial.dtr());
  clearDisplay();
  drawHeader("Reflow", -1, MAX31855_NO_ERR);
}

elapsedMillis m;
uint32_t numS = 0;

void loop() {
  bool touchedLearn = false,
    touchedBake = false,
    touchedReflow = false,
    onSecondInterval = false;

  if (m/1000 > numS) {
    numS++;
    onSecondInterval = true;
    displayTemp(tc.temp, tc.err);
  } else {
    onSecondInterval = false;
  }

  if(touchedLearn) {
    tcTimer.end();
    learn();
  } else if(touchedBake) {
    tcTimer.begin(tcRead, THERMOCOUPLE_READ_INTERVAL_MILLIS * 1000);
    bake();
  } else if(touchedReflow) {
    tcTimer.begin(tcRead, THERMOCOUPLE_READ_INTERVAL_MILLIS * 1000);
    reflow();
  }
}
