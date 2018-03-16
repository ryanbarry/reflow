#ifndef _DISPLAY_H
#define _DISPLAY_H

//LCD controller
#define LIGHT_GREY ((0x8<<11)|(0x10<<5)|0x8)
#define TFT_DC 9
#define TFT_CS 10

#include "Adafruit_ILI9341.h"
#include <Adafruit_GFX.h>
#define HEADER_FONT FreeSansBoldOblique12pt7b
#include <Fonts/FreeSansBoldOblique12pt7b.h>

void setupDisplay(void);
void clearDisplay(void);
void displayTemp(void);
void drawHeader(const char *title);

#endif
