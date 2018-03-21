#include <Arduino.h>

#include "temp.h"
#include "learn.h"
#include "bake.h"
#include "reflow.h"
#include "display.h"
#include "pins.h"

#include "XPT2046_Calibrated.h"
XPT2046_Calibrated ts(T_CS, T_IRQ);

//#define ENABLE_TOUCH_CALIBRATION

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

  ts.begin();
  ts.setRotation(3);
#ifdef ENABLE_TOUCH_CALIBRATION
  ts.runCalibration(tft);
#endif

  ts.calibrate(-0.0882, -0.000174, 329, 0.000238, -0.0668, 257, 320, 240);

  drawHeader("Reflow", -1, MAX31855_NO_ERR);
}

elapsedMillis m;
uint32_t numS = 0;

void loop() {
  bool touchedLearn = false,
    touchedBake = false,
    touchedReflow = false,
    touchedSetup = false,
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
  } else if(touchedSetup) {
    tcTimer.end();
    setup();
  }
}
