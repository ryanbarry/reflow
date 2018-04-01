#include "learn.h"
#include <stdint.h>
#include <Arduino.h>
#include "pins.h"
#include "MAX31855.h"

#define LEARNING_INERTIA_TEMP 150 * 4  // Target temperature to run inertia tests against (in 1/4th degrees Celsius)
#define LEARNING_SOAK_TEMP 120 * 4  // Target temperature to run tests against (in 1/4th degrees Celsius)
#define LEARNING_MAX_TEMP 200 * 4 // temperature not to exceed when running learning cycle

#define LEARNING_INITIAL_DURATION 720  // Duration of the initial "get-to-temperature" phase
#define LEARNING_CONSTANT_TEMP_DURATION 480  // Duration of subsequent constant temperature measurements
#define LEARNING_INERTIA_DURATION 720  // Duration of subsequent temperature change measurements
#define LEARNING_FINAL_INERTIA_DURATION 360  // Duration of the final inertia phase, where temp doesn't need to be stabilized
#define LEARNING_SECONDS_TO_INDICATOR 60  // Seconds after phase start before displaying the performance indicator

typedef enum {
  INITIAL_RAMP = 0,
  CONSTANT_TEMP = 1,
  THERMAL_INERTIA = 2,
  START_COOLING = 3,
  COOLING = 4,
  DONE = 5,
  ABORT = 6
} LearningPhase;

typedef enum {
  WHOLE_OVEN = 0,
  SINGLE_TOP = 1,
  SINGLE_BOTTOM = 2,
  SINGLE_BOOST = 3
} MeasurementMode;

void learn(Thermocouple &tc) {
  //TODO: set up display for this operation
  elapsedMillis lastLoopTime = 0;
  LearningPhase phase = INITIAL_RAMP;
  MeasurementMode mode = WHOLE_OVEN;

  int32_t currentTemp = 0, peakTemp = 0, desiredTemp = LEARNING_INERTIA_TEMP;
  uint8_t elementDutyCounter[NUM_HEATERS];
  uint8_t learningDutyCycle, learningIntegral = 0, coolingDuration = 0;
  uint16_t secondsLeftOfLearning, secondsLeftOfPhase, secondsIntoPhase = 0, secondsTo150C = 0;

  // Initialize varaibles used for learning
  secondsLeftOfLearning = LEARNING_INITIAL_DURATION + (2 * LEARNING_CONSTANT_TEMP_DURATION) + (2 * LEARNING_INERTIA_DURATION) + LEARNING_FINAL_INERTIA_DURATION;
  secondsLeftOfPhase = LEARNING_INITIAL_DURATION;

  // Start with a duty cycle appropriate to the testing temperature
  learningDutyCycle = 60;

  // init all outputs off
  ALL_HEATERS_OFF;
  OUTPUT_OFF(OUTPIN_FAN_COOLING);

  //if (abort) button touched, exit

  for(uint8_t i = 0; i < NUM_HEATERS; i++) {
    elementDutyCounter[i] = (65 * i) % 100; // TODO: does this make sense?
  }

  //TODO: set up buttons for this operation

  while(1) {
    //TODO: check if user hit a (the?) button

    if(lastLoopTime < 20) {
      delay(1);
      continue;
    } else {
      lastLoopTime = 0;
    }

    //TODO: do the following on consecutive loops (not during the same loop)
    //TODO: display countdown timer
    //TODO: display current temperature
    //TODO: send debug info to Serial

    tc.readAll();
    if(tc.err != MAX31855_NO_ERR) {
      // display error on screen
      phase = ABORT;
      ALL_HEATERS_OFF;

      //TODO: wait here till the user taps the button?
      // or maybe just spinlock here, as in, have to unplug power to reset?
    }

    if(tc.temp > LEARNING_MAX_TEMP) {
      //TODO: display error message about max temp exceeded
      // according to Controleo3, this shouldn't happen https://github.com/engineertype/Controleo3/blob/47675e6766cbc2b19967f7d8f255674a3a45db14/examples/ReflowWizard/Learn.ino#L184
      phase = COOLING;
    }

    switch(phase) {
    case INITIAL_RAMP:
      //TODO: only do this once per second (e.g. break if not on secondInterval, or spin here, etc.)
      break;
    case CONSTANT_TEMP:
      break;
    case THERMAL_INERTIA:
      break;
    case START_COOLING:
      break;
    case COOLING:
      break;
    case DONE:
      break;
    case ABORT:
      break;
    }
  }
}
