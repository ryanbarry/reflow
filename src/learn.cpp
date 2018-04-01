#include "learn.h"
#include <stdint.h>
#include <Arduino.h>
#include "pins.h"
#include "MAX31855.h"
#include "display.h"
#include "tones.h"

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

void showLearnedNumbers(uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);

void learn(Thermocouple &tc) {
  //TODO: set up display for this operation
  elapsedMillis lastLoopTimeAgo = 0;
  long lastOverTempTime = 0;
  uint32_t loopCounter = 0;
  bool onSecondInterval = false, shouldHeat = true;
  LearningPhase phase = INITIAL_RAMP;
  MeasurementMode mode = WHOLE_OVEN;

  int32_t currentTemp = 0, peakTemp = 0, desiredTemp = LEARNING_INERTIA_TEMP;
  uint8_t topHeaterDutyCounter = 0, bottomHeaterDutyCounter = 0, boostHeaterDutyCounter = 0,
    topDutyOffset = 0, bottomDutyOffset = 33, boostDutyOffset = 66;
  uint8_t learningDutyCycle, learningIntegral = 0, coolingDuration = 0;
  uint8_t learnedPowerWholeOven=0, learnedPowerTop=0, learnedPowerBottom=0;
  uint16_t secondsLeftOfLearning, secondsLeftOfPhase, secondsIntoPhase = 0, secondsTo150C = 0;
  uint16_t learnedInertiaWholeOven = 0, learnedInertiaTop = 0, learnedInertiaBottom = 0, learnedInsulation = 0;

  // Initialize varaibles used for learning
  secondsLeftOfLearning = LEARNING_INITIAL_DURATION + (2 * LEARNING_CONSTANT_TEMP_DURATION) + (2 * LEARNING_INERTIA_DURATION) + LEARNING_FINAL_INERTIA_DURATION;
  secondsLeftOfPhase = LEARNING_INITIAL_DURATION;

  // Start with a duty cycle appropriate to the testing temperature
  learningDutyCycle = 60;

  // init all outputs off
  ALL_HEATERS_OFF;
  OUTPUT_OFF(PIN_FAN_COOLING);

  //if (abort) button touched, exit

  //TODO: set up buttons for this operation

  clearDisplay();
  drawHeader("Learning", tc.avgtmp, tc.err);

  drawPerformanceBar(true, NO_PERFORMANCE_INDICATOR);

  while(1) {
    //TODO: check if user hit a (the?) button

    // visual indicator of shouldHeat
    if(shouldHeat) {
      tft.fillRect(160-10, 2, 20, 20, ILI9341_RED);
    } else {
      tft.fillRect(160-10, 2, 20, 20, ILI9341_BLUE);
    }

    if(lastLoopTimeAgo < 20) {
      delay(1);
      continue;
    } else {
      lastLoopTimeAgo = 0;
    }

    loopCounter++;
    onSecondInterval = (0 == loopCounter % 50);

    if(tc.err != MAX31855_NO_ERR) {
      Serial.print("thermocouple error! [");
      switch(tc.err) {
      case MAX31855_ERR_OC:
        Serial.print("N/C");
        break;
      case MAX31855_ERR_SCV:
        Serial.print("shHi");
        break;
      case MAX31855_ERR_SCG:
        Serial.print("shLo");
        break;
      }
      Serial.println("]");
      //TODO: display error on screen
      //phase = ABORT;
      //ALL_HEATERS_OFF;

      //TODO: wait here till the user taps the button?
      // or maybe just spinlock here, as in, have to unplug power to reset?
    } else {
      currentTemp = tc.avgtmp;
    }

    //TODO: do the following on consecutive loops (not during the same loop)
    if (loopCounter % 50 == 0) {
      tft.setFont();
      tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
      tft.setCursor(10, 200);
      tft.print("Time left overall: "); tft.printf("%04d", secondsLeftOfLearning);
      tft.setCursor(10, 220);
      tft.print("In phase: "); tft.printf("%03d", secondsLeftOfPhase);
    } else if(loopCounter % 50 == 2) {
      displayTemp(currentTemp, tc.err);
    } else if(loopCounter % 50 == 5) {
      //TODO: send debug info to Serial
    }

    if(tc.avgtmp > LEARNING_MAX_TEMP) {
      Serial.println("Skipping to COOLING phase because max temp exceeded!");
      //TODO: display error message about max temp exceeded
      // according to Controleo3, this shouldn't happen https://github.com/engineertype/Controleo3/blob/47675e6766cbc2b19967f7d8f255674a3a45db14/examples/ReflowWizard/Learn.ino#L184
      phase = COOLING;
    }

    switch(phase) {
    case INITIAL_RAMP:
      if(!onSecondInterval) {
        break;
      }

      secondsLeftOfLearning--;
      secondsLeftOfPhase--;

#define DIFFTEMP (LEARNING_SOAK_TEMP - currentTemp)

      // Reduce the duty cycle as the oven closes in on the desired temperature
      if (DIFFTEMP < (40*4) && DIFFTEMP > (20*4)) {
        learningDutyCycle = 30;
      } else if (DIFFTEMP < (20*4)) {
        phase = CONSTANT_TEMP;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
        Serial.println("learningPhase -> LEARNING_PHASE_CONSTANT_TEMP");
#endif
        learningDutyCycle = 15;
        secondsIntoPhase = 0;
      }
      if (secondsLeftOfPhase < 480) {
        // Should not be in this phase with less than 8 minutes left of the phase
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
        Serial.println("Could not heat up fast enough!");
#endif
        phase = START_COOLING;
      }
      break;
    case CONSTANT_TEMP:
      if(!onSecondInterval) {
        break;
      }

      secondsLeftOfLearning--;
      secondsLeftOfPhase--;
      secondsIntoPhase++;

      // Give the user some indiction of how the oven is performing
      if (secondsIntoPhase > LEARNING_SECONDS_TO_INDICATOR) {
        switch(mode) {
        case WHOLE_OVEN:
          // (12% = excellent, 35% = awful)
          drawPerformanceBar(false, map(constrain(learningDutyCycle, 12, 35), 12, 35, 100, 0));
          break;
        case SINGLE_BOTTOM:
          // (28% = excellent, 50% = awful)
          drawPerformanceBar(false, map(constrain(learningDutyCycle, 28, 50), 28, 50, 100, 0));
          break;
        case SINGLE_TOP:
          // (25% = excellent, 45% = awful)
          // Closer to thermocouple, heating less of the oven = less power needed
          drawPerformanceBar(false, map(constrain(learningDutyCycle, 25, 45), 25, 45, 100, 0));
          break;
        default:
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("switch handling undefined case! [top of CONSTANT_TEMP]");
#endif
        }
      }

      // Is the oven too hot?
      if (currentTemp > LEARNING_SOAK_TEMP) {
        if(shouldHeat) {
          shouldHeat = false;
          ALL_HEATERS_OFF;

          // The duty cycle caused the temperature to exceed the bake temperature, so decrease it
          // (but not more than once every 30 seconds)
          if (millis() - lastOverTempTime > (30 * 1000)) {
            lastOverTempTime = millis();
            if (learningDutyCycle > 0)
              learningDutyCycle--;
          }

          // Reset the bake integral, so it will be slow to increase the duty cycle again
          learningIntegral = 0;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("Over LEARNING_SOAK_TEMP. Elements off");
#endif
        }
      } else {
        shouldHeat = true;

        // Increase the bake integral if not close to temperature
        if ((LEARNING_SOAK_TEMP - currentTemp) > (1*4)) {
          learningIntegral++;
        } else if ((LEARNING_SOAK_TEMP - currentTemp) > (5*4)) {
          learningIntegral++;
        }
          
        // Has the oven been under-temperature for a while?
        if (learningIntegral > 30) {
          learningIntegral = 0;
          if (learningDutyCycle < 100) {
            // Increase duty cycles
            learningDutyCycle++;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
            Serial.print("Under-temp. Increasing duty cycle to "); Serial.println(learningDutyCycle);
#endif
          }
        }
      }

      // Time to end this phase?
      if (secondsLeftOfPhase == 0) {
        secondsLeftOfPhase = LEARNING_CONSTANT_TEMP_DURATION;
        // Move to the next phase
        drawPerformanceBar(false, NO_PERFORMANCE_INDICATOR);
        switch(mode) {
        case WHOLE_OVEN:
          learnedPowerWholeOven = learningDutyCycle;
          Serial.print("learned power (WHOLE_OVEN) = "); Serial.println(learningDutyCycle);
          mode = SINGLE_BOTTOM;
          learningPrintln("Keeping oven at 120~C using just the bottom element");
          // The duty cycle for just the bottom element is probably twice the whole oven
          learningDutyCycle = (learningDutyCycle << 1) + 5;
          break;
        case SINGLE_BOTTOM:
          learnedPowerBottom = learningDutyCycle;
          Serial.print("learned power (SINGLE_BOTTOM) = "); Serial.println(learningDutyCycle);
          mode = SINGLE_TOP;
          learningPrintln("Keeping oven at 120~C using just the top element");
          // The duty cycle for just the top element is probably slightly higher than the bottom one
          learningDutyCycle = learningDutyCycle + 2;
          break;
        case SINGLE_TOP:
          learnedPowerTop = learningDutyCycle;
          Serial.print("learned power (SINGLE_TOP) = "); Serial.println(learningDutyCycle);
          // Time to measure the thermal intertia now
          learningPrintln("Measuring how quickly the oven gets to 150~C using all elements at 80%");
          phase = THERMAL_INERTIA;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("phase -> THERMAL_INERTIA");
#endif
          mode = WHOLE_OVEN;
          secondsLeftOfPhase = LEARNING_INERTIA_DURATION;
          // Crank up the power to 80% to see the rate-of-rise
          learningDutyCycle = 80;
          secondsTo150C = 0;
          peakTemp = 0;
          //prefs.learnedInsulation = 0;
          break;
        default:
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("switch handling undefined case [bottom of CONSTANT_TEMP]");
#endif
          break;
        }
        // Reset some phase variables
        secondsIntoPhase = 0;
      }
      break;
    case THERMAL_INERTIA:
      if(!onSecondInterval) break;

      secondsLeftOfLearning--;
      secondsLeftOfPhase--;
      secondsIntoPhase++;

      // Save the peak temperature
      if (currentTemp > peakTemp) {
        peakTemp = currentTemp;
        // Has the temperature passed 150C yet?
        if (secondsTo150C == 0 && currentTemp >= 150.0) {
          secondsTo150C = secondsIntoPhase;
          // Return to the soak temperature
          desiredTemp = LEARNING_SOAK_TEMP;
          // Give the user some indiction of how the oven is performed
          switch(mode) {
          case WHOLE_OVEN:
            // (35 seconds = excellent, 60 seconds = awful)
            drawPerformanceBar(false, map(constrain(secondsTo150C, 35, 60), 35, 60, 100, 0));
            break;
          case SINGLE_BOTTOM:
            // (120 seconds = excellent, 240 seconds = awful)
            drawPerformanceBar(false, map(constrain(learningDutyCycle, 100, 240), 100, 240, 100, 0));
            break;
          case SINGLE_TOP:
            // (60 seconds = excellent, 140 seconds = awful)
            // Closer to thermocouple, heating less of the oven = less time needed
            drawPerformanceBar(false, map(constrain(learningDutyCycle, 60, 140), 60, 140, 100, 0));
            break;
          default:
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
            Serial.println("switch handling undefined case (thermal inertia)");
#endif
          }
          // Set the duty cycle at the value that will maintain soak temperature
          //learningDutyCycle = prefs.learnedPower[currentlyMeasuring];
          switch(mode) {
          case WHOLE_OVEN:
            learningDutyCycle = learnedPowerWholeOven;
            break;
          case SINGLE_TOP:
            learningDutyCycle = learnedPowerTop;
            break;
          case SINGLE_BOTTOM:
            learningDutyCycle = learnedPowerBottom;
            break;
          default:
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
            Serial.println("switch handling undefined case (thermal inertia #2)");
#endif
            break;
          }
          
          // Stop heating the oven
          shouldHeat = false;
          // Turn all heating elements off
          ALL_HEATERS_OFF;

          // Update the message to show the oven is trying to stabilize around 120C again
          learningPrintln("Cooling oven back to 120~C and measuring heat retention");
        }
      }

      // Time the drop from 150C to 120C.  We can do this when the target temperature is 120C
      if (desiredTemp == LEARNING_SOAK_TEMP) {
        if (currentTemp > LEARNING_INERTIA_TEMP) {
          // If the temp is still above 150C then reset the timer
          secondsIntoPhase = 0;
        }
        if (currentTemp < LEARNING_SOAK_TEMP && learnedInsulation == 0 && mode == WHOLE_OVEN) {
          // If the temperature has dropped below 120C then record the time it took
          //prefs.learnedInsulation = secondsIntoPhase;
          learnedInsulation = secondsIntoPhase;
          Serial.print("learned insulation = "); Serial.println(learnedInsulation);
        }
        if (mode == SINGLE_TOP) {
          // If we're done measuring everything then abort this phase early
          secondsLeftOfLearning = 0;
          secondsLeftOfPhase = 0;
        }
      }

      // Time to end this phase?
      if (secondsLeftOfPhase == 0) {
        // Save the time taken to reach 150C

        //prefs.learnedInertia[currentlyMeasuring++] = secondsTo150C;
        switch(mode) {
        case WHOLE_OVEN:
          learnedInertiaWholeOven = secondsTo150C;
          Serial.print("learned inertia (WHOLE_OVEN) = "); Serial.println(secondsTo150C);
          mode = SINGLE_BOTTOM;
          break;
        case SINGLE_BOTTOM:
          learnedInertiaBottom = secondsTo150C;
          Serial.print("learned inertia (SINGLE_BOTTOM) = "); Serial.println(secondsTo150C);
          mode = SINGLE_TOP;
          break;
        case SINGLE_TOP:
          learnedInertiaTop = secondsTo150C;
          Serial.print("learned inertia (SINGLE_TOP) = "); Serial.println(secondsTo150C);
          mode = SINGLE_BOOST; // not actually used, see default case
          break;
        default:
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
            Serial.println("switch handling undefined case (thermal inertia #3)");
#endif
        }
        // Move to the next phase
        drawPerformanceBar(true, NO_PERFORMANCE_INDICATOR);
        switch(mode) {
        case SINGLE_BOTTOM:
          learningPrintln("Measuring how quickly the oven gets to 150~C using bottom element at 80%.");
          secondsLeftOfPhase = LEARNING_INERTIA_DURATION;
          learningDutyCycle = 80;
          break;
        case SINGLE_TOP:
          // The top element is closer to the thermocouple, and is also close to the insulation so run it cooler
          learningPrintln("Measuring how quickly the oven gets to 150~C using the top element at 70%.");
          // No need to stabilize temperature at 120C at the end of this phase (so it has shorter duration)
          secondsLeftOfPhase = LEARNING_FINAL_INERTIA_DURATION;
          learningDutyCycle = 70;
          break;
        default:
          // Erase the bottom part of the screen
          tft.fillRect(0, 110, SCREEN_WIDTH, 120, ILI9341_WHITE);
          // Show the results now
          showLearnedNumbers(learnedPowerWholeOven, learnedPowerBottom, learnedPowerTop, learnedInertiaWholeOven, learnedInertiaBottom, learnedInertiaTop, learnedInsulation);
          // Done with measuring the oven.  Start cooling
          phase = START_COOLING;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("phase -> START_COOLING");
#endif
          secondsLeftOfPhase = 0;
          // Save all the learned values now
          //prefs.learningComplete = true;
          //savePrefs();
          break;
        }
        // Reset some phase variables
        peakTemp = 0;
        secondsIntoPhase = 0;
        secondsTo150C = 0;
        desiredTemp = LEARNING_INERTIA_TEMP;
        break;
      }

      // Is the oven too hot, and we're trying to settle around the lower soak temperature, and we're heating the oven?
      if (currentTemp > desiredTemp && desiredTemp == LEARNING_SOAK_TEMP && shouldHeat) {
        shouldHeat = false;
        // Turn all heating elements off
        ALL_HEATERS_OFF;

        // The duty cycle caused the temperature to exceed the soak temperature, so decrease it
        // (but not more than once every 30 seconds)
        if (millis() - lastOverTempTime > (30 * 1000)) {
          lastOverTempTime = millis();
          if (learningDutyCycle > 0)
            learningDutyCycle--;
        }

        learningIntegral = 0;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
        Serial.println("Over-temp. Elements off");
#endif
      } else if (currentTemp < desiredTemp) {
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
        Serial.print("heating ");
#endif
        // Is the oven below temperature?
        // The oven is heating up
        shouldHeat = true;

        // Increase the bake integral if not close to temperature and we're trying to settle around the lower soak temperature
        if (LEARNING_SOAK_TEMP - currentTemp > (1*4) && desiredTemp == LEARNING_SOAK_TEMP) {
          learningIntegral++;
        }
          
        // Has the oven been under-temperature for a while?
        if (learningIntegral > 30) {
          learningIntegral = 0;
          // Increase duty cycles
          if (learningDutyCycle < 100) {
            learningDutyCycle++;
          }
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
          Serial.println("Under-temp. Increasing duty cycle");
#endif          
        }
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
        else { Serial.println(); }
#endif
      }

      break;
    case START_COOLING:
      shouldHeat = false;
      
      // Turn off all elements and turn on the fans
      ALL_HEATERS_OFF;
     
      // Move to the next phase
      phase = COOLING;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
      Serial.println("phase -> COOLING");
#endif

      // If a servo is attached, use it to open the door over 10 seconds
      //setServoPosition(prefs.servoOpenDegrees, 10000);

      // Change the STOP button to DONE
      //tft.fillRect(150, 242, 180, 36, WHITE);
      //drawButton(110, 230, 260, 93, BUTTON_LARGE_FONT, (char *) "DONE");
        
      // Cooling should be at least 60 seconds in duration
      coolingDuration = 60;
        
      break;
    case COOLING:
       // Make changes every second
      if (!onSecondInterval) break;
          
      // Wait in this phase until the oven has cooled
      if (coolingDuration > 0)
        coolingDuration--;      
      if (currentTemp < 50*4 && coolingDuration == 0) {
        shouldHeat = false;
        // Turn all elements and fans off
        //setOvenOutputs(ELEMENTS_OFF, CONVECTION_FAN_OFF, COOLING_FAN_OFF);
        // Close the oven door now, over 3 seconds
        //setServoPosition(prefs.servoClosedDegrees, 3000);
        // Stay on this screen and wait for the user tap
        phase = DONE;
#if defined(DEBUG_ALL) || defined(DEBUG_LEARN)
      Serial.println("phase -> DONE");
#endif
        // Play a tune to let the user know baking is done
        playTones(tuneReflowBeep);
      }
      break;
    case DONE:
      
      break;
    case ABORT:
      SerialUSB.println("Learning is over!");
      // Turn all elements and fans off
      ALL_HEATERS_OFF;
      //setOvenOutputs(ELEMENTS_OFF, CONVECTION_FAN_OFF, COOLING_FAN_OFF);
      // Close the oven door now, over 3 seconds
      //setServoPosition(prefs.servoClosedDegrees, 3000);
      // Return to the main menu
      break;
    }

    // Turn the outputs on or off based on the duty cycle
    if (shouldHeat) {
      if (topHeaterDutyCounter == topDutyOffset && (mode == WHOLE_OVEN || mode == SINGLE_TOP)) {
        OUTPUT_ON(PIN_HEATER_TOP);
      } else if (topHeaterDutyCounter == ((learningDutyCycle < 80? learningDutyCycle: 80) + topDutyOffset)%100) {
        // Restrict the top element's duty cycle to 80% to protect the insulation
        // and reduce IR heating of components
        OUTPUT_OFF(PIN_HEATER_TOP);
      }

      if (bottomHeaterDutyCounter == bottomDutyOffset && (mode == WHOLE_OVEN || mode == SINGLE_BOTTOM)) {
        OUTPUT_ON(PIN_HEATER_BOTTOM);
      } else if (bottomHeaterDutyCounter == ((learningDutyCycle + bottomDutyOffset)%100)) {
        OUTPUT_OFF(PIN_HEATER_BOTTOM);
      }

      // boost gets half the duty cycle of the other heaters
      if (boostHeaterDutyCounter == boostDutyOffset && mode == WHOLE_OVEN) {
        OUTPUT_ON(PIN_HEATER_BOOST);
      } else if (boostHeaterDutyCounter == ((learningDutyCycle/2 + boostDutyOffset)%100)) {
        OUTPUT_OFF(PIN_HEATER_BOOST);
      }
    }
      
    // Increment the duty counters
    topHeaterDutyCounter = (topHeaterDutyCounter + 1) % 100;
    bottomHeaterDutyCounter = (bottomHeaterDutyCounter + 1) % 100;
    boostHeaterDutyCounter = (boostHeaterDutyCounter + 1) % 100;
  }
}

uint8_t ovenScore(uint8_t learnedPowerWholeOven, uint16_t learnedInertiaWholeOven, uint16_t learnedInsulation) {
  uint8_t score;

  // Ability to hold temperature using very little power is worth 40%
  // 12% (or less) duty cycle scores all 40 points, sliding to 0 if required power is 30%
  score = map(constrain(learnedPowerWholeOven, 12, 30), 12, 30, 40, 0);

  // Thermal inertia is worth another 40%
  // Taking 36 seconds (or less) to gain 30C scores all the points, sliding to 0 if it takes 60 seconds or longer
  score += map(constrain(learnedInertiaWholeOven, 36, 60), 36, 60, 40, 0);

  // The remaining 20% is allocated towards insulation
  // A well-insulated oven will lose 30C in just over 2 minutes, and a poorly insulated one in 80 seconds
  score += map(constrain(learnedInsulation, 80, 130), 80, 130, 0, 20);

  // And that is the oven score!
  return score;
}

// The learned numbers are shown once the oven has completed the 1-hour learning run
void showLearnedNumbers(uint8_t learnedPowerWholeOven, uint8_t learnedPowerBottom, uint8_t learnedPowerTop, uint16_t learnedInertiaWholeOven, uint16_t learnedInertiaBottom, uint16_t learnedInertiaTop, uint16_t learnedInsulation) {
  uint16_t score;
  // Display the power required to keep the oven at a stable temperature
  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor(5, 50);
  tft.printf("Power: %d%% (bottom %d%%, top %d%%)", learnedPowerWholeOven, learnedPowerBottom, learnedPowerTop);
  // Show the emoticon that corresponds to the power
  //renderBitmap(prefs.learnedPower[0] < 18? BITMAP_SMILEY_GOOD : prefs.learnedPower[0] < 24? BITMAP_SMILEY_NEUTRAL: BITMAP_SMILEY_BAD, offset, LINE(0)-3);
  // Add the width of the emoticon, plus some space

  // Display the time needed to reach the higher temperatures (inertia)
  tft.setCursor(10, 80);
  tft.printf("Inertia: %ds (bottom %ds, top %ds)", learnedInertiaWholeOven, learnedInertiaBottom, learnedInertiaTop);
  // Show the emoticon that corresponds to the inertia
  //renderBitmap(prefs.learnedInertia[0] < 46? BITMAP_SMILEY_GOOD : prefs.learnedInertia[0] < 56? BITMAP_SMILEY_NEUTRAL: BITMAP_SMILEY_BAD, offset, LINE(1)-3);

  // Display the insulation score
  tft.setCursor(10, 110);
  tft.printf("Insulation: %ds", learnedInsulation);
  // Show the emoticon that corresponds to the insulation
  //renderBitmap(prefs.learnedInsulation > 105? BITMAP_SMILEY_GOOD : prefs.learnedInsulation > 85? BITMAP_SMILEY_NEUTRAL: BITMAP_SMILEY_BAD, offset, LINE(2)-3);


  // Display the overall oven score
  tft.setCursor(10, 140);
  score = ovenScore(learnedPowerWholeOven, learnedInertiaWholeOven, learnedInsulation);
  tft.printf("Oven score: %d%% = ", score);
  if(score > 90) tft.print("Excellent");
  else if(score > 80) tft.print("Very good");
  else if(score > 70) tft.print("Good");
  else if(score > 60) tft.print("Marginal");
  else tft.print("Not good.");
}
