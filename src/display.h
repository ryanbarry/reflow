#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "Adafruit_ILI9341.h"
#include <Adafruit_GFX.h>
#define HEADER_FONT FreeSansBoldOblique12pt7b
#include <Fonts/FreeSansBoldOblique12pt7b.h>

#include "pins.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define BUTTON_HEIGHT 40 // pixels
#define BUTTON_WIDTH 100
#define BUTTON_VERT_SPACING (BUTTON_HEIGHT+10)

extern Adafruit_ILI9341 tft;

void setupDisplay(void);
void clearDisplay(void);

void displayTemp(int32_t, uint8_t);
void drawHeader(const char*, int32_t, uint8_t);

#define NO_PERFORMANCE_INDICATOR      101     // Don't draw an indicator on the performance bar
void drawPerformanceBar(boolean redraw, uint8_t percentage);
void learningPrintln(const char* s);

#endif
