#include <Arduino.h>

#include "temp.h"
#include "learn.h"
#include "bake.h"
#include "reflow.h"
#include "display.h"

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
  bool touchedLearn = false,
    touchedBake = false,
    touchedReflow = false;//,
    //onSecondInterval = false;

  if (m/1000 > numS) {
    numS++;
    //onSecondInterval = true;
    displayTemp();
  } else {
    //onSecondInterval = false;
  }

  if(touchedLearn) learn();
  else if(touchedBake) bake();
  else if(touchedReflow) reflow();
}
