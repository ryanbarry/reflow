#include "temp.h"
#include "MAX31855.h"
#include "display.h"

Adafruit_ILI9341 tft(PIN_TFT_CS, PIN_TFT_DC); // only specifying cs & dc means use HW SPI

void setupDisplay(void) {
  tft.begin();
  tft.setRotation(3);
}

void clearDisplay(void) {
  tft.fillScreen(ILI9341_WHITE);
}

void drawHeader(const char *title, int32_t temp, uint8_t tcerr) {
  tft.fillRect(0, 0, 320, 28, ILI9341_WHITE);
  tft.fillRect(8, 24, 304, 4, ILI9341_BLUE);
  tft.setTextSize(1);
  tft.setCursor(10, 13);
  tft.setFont(&HEADER_FONT);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(title);
  displayTemp(temp, tcerr);
}

void displayTemp(int32_t temp, uint8_t tcerr)  {
  tft.setFont(&HEADER_FONT);
  tft.setCursor(240, 18);
  tft.setTextSize(1);
  if (tcerr != MAX31855_NO_ERR) {
    tft.fillRect(240, 0, 80, 20, ILI9341_BLACK);
    tft.setTextColor(ILI9341_MAGENTA);
    switch (tcerr) {
    case MAX31855_ERR_OC:
      tft.print("N/C");
      break;
    case MAX31855_ERR_SCV:
      tft.print("shHi");
      break;
    case MAX31855_ERR_SCG:
      tft.print("shLo");
      break;
    }
  } else if (temp >= 200) {
    tft.fillRect(240, 0, 80, 20, ILI9341_WHITE);
    tft.setTextColor(ILI9341_RED);
    tft.print(temp >> 2);
  } else {
    tft.fillRect(240, 0, 80, 20, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.print(temp >> 2);
  }
  switch (temp & 0x3) {
  case 0:
    tft.print(".00");
    break;
  case 1:
    tft.print(".25");
    break;
  case 2:
    tft.print(".50");
    break;
  case 3:
    tft.print(".75");
    break;
  }
}

#define PERFORMANCE_BAD               0xF082  // (tft.convertTo16Bit(0xF01111))
#define PERFORMANCE_OKAY              0x09BF  // (tft.convertTo16Bit(0x0D35F9))
#define PERFORMANCE_GOOD              0x5F0B  // (tft.convertTo16Bit(0x5EE05F))
void drawPerformanceBar(boolean redraw, uint8_t percentage)
{
  static uint16_t lastStatusX = 0;

  const int xoffset = 20, yoffset = 110, barwidth = SCREEN_WIDTH - (xoffset*2), barheight = 20;

  if (redraw) {
    // The status bar must be redrawn
    // Erase the space that the bar occupies
    tft.fillRect(xoffset, yoffset, barwidth, barheight, ILI9341_WHITE);
    // Draw the color strips
    tft.fillRect(xoffset, yoffset+2, barwidth/3, barheight-4, PERFORMANCE_BAD);
    tft.fillRect(xoffset+barwidth/3, yoffset+2, barwidth/3, barheight-4, PERFORMANCE_OKAY);
    tft.fillRect(xoffset+barwidth/3*2, yoffset+2, barwidth/3, barheight-4, PERFORMANCE_GOOD);
    lastStatusX = 0;
  } else {
    // Erase the mark, if there was one
    if (lastStatusX) {
      // Draw white at the top and bottom
      tft.fillRect(lastStatusX-3, yoffset, 7, 2, ILI9341_WHITE);
      tft.fillRect(lastStatusX-3, yoffset+barheight-2, 7, 2, ILI9341_WHITE);
      // Restore the color to the bar
      for (uint16_t i= lastStatusX-3; i <= lastStatusX+3; i++)
        tft.fillRect(i, yoffset+2, 1, barheight-4, i < (xoffset+barwidth/3)? PERFORMANCE_BAD: i >= (xoffset+barwidth/3*2)? PERFORMANCE_GOOD: PERFORMANCE_OKAY);
    }

    // Draw the mark at the new location
    if (percentage <= 100) {
      lastStatusX = map(percentage, 0, 100, xoffset+1, xoffset+barwidth-1);
      tft.fillRect(lastStatusX-3, yoffset, 7, barheight, ILI9341_BLACK);
      tft.fillRect(lastStatusX-2, yoffset+1, 5, barheight-2, ILI9341_WHITE);
    }
  }
}

void learningPrintln(const char* s) {
  tft.fillRect(5, 50, SCREEN_WIDTH-5, 20, ILI9341_WHITE); // clear text & perf bar
  tft.setFont();
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor(5, 50);
  tft.setTextSize(1);
  tft.print(s);
}
