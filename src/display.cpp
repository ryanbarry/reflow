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
      tft.print("ShVcc");
      break;
    case MAX31855_ERR_SCG:
      tft.print("ShGND");
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
}
