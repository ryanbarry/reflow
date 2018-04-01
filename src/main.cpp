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
Adafruit_GFX_Button b1, b2, b3, b4;
btn buttons[] = {
  {&b1, "Top", topHeaterToggle},
  {&b2, "Bottom", botHeaterToggle},
  {&b3, "Boost", bstHeaterToggle},
  {&b4, "Setup", doSetup}
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

  drawHeader("Reflow", -1, MAX31855_NO_ERR);

  tft.setFont();
#if defined(DEBUG_ALL) || defined(DEBUG_TFT)
  Serial.print("init'ing & drawing "); Serial.print(numButtons); Serial.println(" buttons");
#endif
  for(int i=0; i < numButtons; i++) {
    buttons[i].button->initButtonUL(&tft, 115, BUTTON_HEIGHT+BUTTON_VERT_SPACING*i,
                                    BUTTON_WIDTH, BUTTON_HEIGHT,
                                    ILI9341_BLACK, ILI9341_WHITE, ILI9341_BLACK,
                                    (char*)buttons[i].name, 2);
    buttons[i].button->drawButton();
  }
#if defined(DEBUG_ALL) || defined(DEBUG_TFT)
  Serial.println("moving on");
#endif

  tcTimer.begin(tcReadIsr, THERMOCOUPLE_READ_INTERVAL_MILLIS * 1000);
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
#if defined(DEBUG_ALL) || defined(DEBUG_TC)
    Serial.print("tc readings: ");
    Serial.println(tcReadingCounter);
#endif
  } else {
    onSecondInterval = false;
  }

  if ((m - lastTouch) > 250) {
    if (ts.touched()) {
      lastTouch = m;
      uint16_t rawX, rawY;
      int32_t touchX, touchY;
      uint8_t touchZ;
      ts.readData(&rawX, &rawY, &touchZ);
      touchX = map(rawX, 380, 3870, 0, 319);
      touchY = map(rawY, 270, 3800, 0, 239);

#if defined(DEBUG_ALL) || defined(DEBUG_TOUCH)
      Serial.printf("touched! (%d, %d, %d, %d, %d)\n", touchX, touchY, touchZ, rawX, rawY);
#endif

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
#if defined(DEBUG_ALL) || defined(DEBUG_TOUCH)
        Serial.print("hit "); Serial.print(buttons[i].name); Serial.println("!");
#endif
        buttons[i].action();
      } else if(buttons[i].button->justReleased()) {
        buttons[i].button->drawButton(false);
      }
    }
  }
}
