#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "Adafruit_ILI9341.h"
#include <Adafruit_GFX.h>
#define HEADER_FONT FreeSansBoldOblique12pt7b
#include <Fonts/FreeSansBoldOblique12pt7b.h>

#include "pins.h"

void setupDisplay(void);
void clearDisplay(void);
void displayTemp(int32_t temp, uint8_t tcerr);
void drawHeader(const char *title, int32_t temp, uint8_t tcerr);

#endif
