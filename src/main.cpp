#include <Arduino.h>
#include "MAX31855.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_GFX.h>
#define HEADER_FONT FreeSansBoldOblique12pt7b
#include <Fonts/FreeSansBoldOblique12pt7b.h>

//MAX38155 thermocouple interface
#define M_CLK 13 //SCK0
#define M_CS 6
#define M_DO 12  //MISO0

//LCD controller
#define LIGHT_GREY ((0x8<<11)|(0x10<<5)|0x8)
#define TFT_DC 9
#define TFT_CS 10

//Touch controller
#define T_CS  8
#define T_IRQ 4


MAX31855 tc(M_CLK, M_CS, M_DO);
Adafruit_ILI9341 tft(TFT_CS, TFT_DC); // only specifying cs & dc means use HW SPI

volatile int32_t temp, tcint;
volatile uint8_t tcerr;

IntervalTimer tcTimer;

void displayTemp() {
  tft.setCursor(240, 18);
  tft.setFont(&HEADER_FONT);
  tcerr = MAX31855_NO_ERR;
  if (tcerr != MAX31855_NO_ERR) {
    tft.fillRect(240, 0, 80, 20, ILI9341_BLACK);
    tft.setTextColor(ILI9341_CYAN);
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
  } else if (temp >= 50.0) {
    tft.fillRect(240, 0, 80, 20, ILI9341_WHITE);
    tft.setTextColor(ILI9341_RED);
    //tft.print(temp >> 2);
    tft.print(tcint >> 4);
  } else {
    tft.fillRect(240, 0, 80, 20, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    //tft.print(temp >> 2);
    tft.print(tcint >> 4);
  }
}

void drawHeader(const char *title) {
  tft.fillRect(0, 0, 320, 28, ILI9341_WHITE);
  tft.fillRect(8, 24, 304, 4, ILI9341_BLUE);
  tft.setCursor(10, 13);
  tft.setFont(&HEADER_FONT);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(title);
  displayTemp();
}

void readTcIsr() {
  tcint = tc.read16thDegInternal();
  temp = tc.readQtrDegCelsius();
  tcerr = tc.readError();
}

void setup() {
  Serial.begin(0);
  tft.begin();
  tft.setRotation(3);
  tc.begin();

  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  
  tft.println("waiting for serial connection...");
  //while (!Serial.dtr());
  tft.fillScreen(ILI9341_WHITE);
  drawHeader("Reflow");

  tcTimer.begin(readTcIsr, 200000);
}

elapsedMillis m;
uint32_t numS = 0;

void loop() {
  if (m/1000 > numS) {
    numS++;
    displayTemp();
  }
}
