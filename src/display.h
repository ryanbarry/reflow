#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "Adafruit_ILI9341.h"
#include <Adafruit_GFX.h>
#define HEADER_FONT FreeSansBoldOblique12pt7b
#include <Fonts/FreeSansBoldOblique12pt7b.h>

#include "pins.h"

#define BUTTON_HEIGHT 40 // pixels
#define BUTTON_WIDTH 100
#define BUTTON_VERT_SPACING (BUTTON_HEIGHT+10)

extern Adafruit_ILI9341 tft;

void setupDisplay(void);
void clearDisplay(void);

void displayTemp(int32_t, uint8_t);
void drawHeader(const char*, int32_t, uint8_t);

Adafruit_GFX_Button& drawButton(uint16_t, uint16_t, uint8_t, uint8_t, const char*);

#endif
