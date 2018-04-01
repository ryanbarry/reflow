#include <Arduino.h>

#include "temp.h"
#include "learn.h"
#include "bake.h"
#include "reflow.h"
#include "setup.h"
#include "display.h"
#include "pins.h"

#include <XPT2046_Touchscreen.h>
XPT2046_Touchscreen ts(PIN_T_CS);

#define THERMOCOUPLE_READ_INTERVAL_MILLIS 200

IntervalTimer tcTimer;
Thermocouple tc;
volatile uint32_t tcReadingCounter = 0;

void tcRead(void) {
  tc.readAll();
  tcReadingCounter++;
}

void nopCb(void) {
  Serial.println("NOP!");
}

void botHeaterToggle(void) {
  static bool on = false;
  if (on) {
    Serial.println("bottom heater off");
    OUTPUT_OFF(PIN_HEATER_BOTTOM);
  } else {
    Serial.println("bottom heater on");
    OUTPUT_ON(PIN_HEATER_BOTTOM);
  }
  on = !on;
}
void topHeaterToggle(void) {
  static bool on = false;
  if (on) {
    Serial.println("top heater off");
    OUTPUT_OFF(PIN_HEATER_TOP);
  } else {
    Serial.println("top heater on");
    OUTPUT_ON(PIN_HEATER_TOP);
  }
  on = !on;
}
void bstHeaterToggle(void) {
  static bool on = false;
  if (on) {
    Serial.println("boost heater off");
    OUTPUT_OFF(PIN_HEATER_BOOST);
  } else {
    Serial.println("boost heater on");
    OUTPUT_ON(PIN_HEATER_BOOST);
  }
  on = !on;
}

typedef struct {
  Adafruit_GFX_Button *button;
  const char *name;
  void (*action) (void);
} btn;
Adafruit_GFX_Button learnButton, bakeButton, reflowButton, setupButton;
btn buttons[] = {
  {&learnButton, "Top", topHeaterToggle},
  {&bakeButton, "Bottom", botHeaterToggle},
  {&reflowButton, "Boost", bstHeaterToggle},
  {&setupButton, "Setup", doSetup}
};
const uint8_t numButtons = sizeof(buttons)/sizeof(btn);

elapsedMillis m;

void setup() {
  Serial.begin(0);

  setupDisplay();
  clearDisplay();
  //drawHeader("Waiting for serial...", -1, MAX31855_NO_ERR);
  while (!Serial.dtr());

  setupPins();

  ts.begin();
  ts.setRotation(1);


#ifdef ENABLE_TOUCH_CALIBRATION
  tft.setFont();
  tft.setTextSize(1);
  ts.runCalibration(tft);
#endif

  drawHeader("Reflow", -1, MAX31855_NO_ERR);

  tft.setFont();
  Serial.print("init'ing & drawing "); Serial.print(numButtons); Serial.println(" buttons");
  for(int i=0; i < numButtons; i++) {
    buttons[i].button->initButtonUL(&tft, 115, BUTTON_HEIGHT+BUTTON_VERT_SPACING*i,
                                    BUTTON_WIDTH, BUTTON_HEIGHT,
                                    ILI9341_BLACK, ILI9341_WHITE, ILI9341_BLACK,
                                    (char*)buttons[i].name, 2);
    buttons[i].button->drawButton();
  }
  Serial.println("moving on");

  tcTimer.begin(tcRead, THERMOCOUPLE_READ_INTERVAL_MILLIS * 1000);
  SPI.usingInterrupt(tcTimer);
  m = 0;
}

uint32_t lastTouch = 0;
uint32_t numS = 0;

void loop() {
  bool onSecondInterval = false;

  if (m/1000 > numS) {
    numS++;
    onSecondInterval = true;
    displayTemp(tc.temp, tc.err);
    //Serial.print("tc readings: ");
    //Serial.println(tcReadingCounter);
  } else {
    onSecondInterval = false;
  }

  if ((m - lastTouch) > 250) {
    if (ts.touched()) {
      lastTouch = m;
      uint16_t rawX, rawY;
      int32_t touchX, touchY;
      uint8_t touchZ;
      //ts.calibRead(&touchX, &touchY, &touchZ, &rawX, &rawY);
      ts.readData(&rawX, &rawY, &touchZ);
      touchX = map(rawX, 380, 3870, 0, 319);
      touchY = map(rawY, 270, 3800, 0, 239);

      Serial.printf("touched! (%d, %d, %d, %d, %d)\n", touchX, touchY, touchZ, rawX, rawY);

      for (int i=0; i < numButtons; i++) {
        if (buttons[i].button->contains(touchX, touchY)) {
          buttons[i].button->press(true);
        } else {
          buttons[i].button->press(false);
        }
      }
    } else {
      for(int i=0; i < numButtons; i++) {
        buttons[i].button->press(false);
      }
    }

    tft.setFont();
    for(int i=0; i < numButtons; i++) {
      if(buttons[i].button->justPressed()) {
        buttons[i].button->drawButton(true);
        Serial.print("hit "); Serial.print(buttons[i].name); Serial.println("!");
        buttons[i].action();
      } else if(buttons[i].button->justReleased()) {
        buttons[i].button->drawButton(false);
      }
    }
  }
}
