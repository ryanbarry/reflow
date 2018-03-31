#include <stdlib.h>
#include <SPI.h>
#include <limits.h>
#include "MAX31855.h"

MAX31855::MAX31855(int8_t _ck, int8_t _cs, int8_t _miso) {
  ck = _ck;
  cs = _cs;
  miso = _miso;

  // MODE0 because data is presented after the end of each clock cycle (and also before the first
  // clock cycle after CS low) to be shifted in on the rising edge of the next
  spiSettings = SPISettings(MAX31855_SPISPEED, MSBFIRST, SPI_MODE0);

  initialized = false;
  lastErr = 0;
}

void MAX31855::begin(void) {
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH); // CS low => enabled, high => disabled

  SPI.begin();

  initialized = true;
}

int32_t MAX31855::read16thDegInternal(void) {
  if (sinceLastRead > 100)
    v = spiread32();

  // ignore bottom 4 bits - they're just thermocouple data
  v >>= 4;

  // pull the bottom 11 bits off
  int32_t internal = v & 0x7FF;
  // check sign bit!
  if (v & 0x800) {
    // Convert to negative value by extending sign and casting to signed type.
    int16_t tmp = 0xF800 | (v & 0x7FF);
    internal = tmp;
  }

  return internal;
}

int32_t MAX31855::readQtrDegCelsius(void) {
  if (sinceLastRead > 100)
    v = spiread32();

  if (v & MAX31855_ERR_BIT) {
    // uh oh, a serious problem!
    return -INT_MAX;
  }

  if (v & 0x80000000) {
    // Negative value, drop the lower 18 bits and explicitly extend sign bits.
    v = 0xFFFFC000 | ((v >> 18) & 0x00003FFFF);
  }
  else {
    // Positive value, just drop the lower 18 bits.
    v >>= 18;
  }
  
  return v;
}

int16_t MAX31855::justReadTc(void) {
  if (!initialized) return MAX31855_ERR_BIT;
  SPI.beginTransaction(spiSettings);

  int16_t d;

  digitalWrite(cs, LOW);
  // tCSS (CS fall to SCK rise) is specified as at least 100ns in the MAX31855 datasheet
  delayMicroseconds(1);

  d = SPI.transfer(0);
  d <<= 8;
  d |= SPI.transfer(0);
  digitalWrite(cs, HIGH);

  SPI.endTransaction();

  return d >> 2;
}

uint8_t MAX31855::readError() {
  return lastErr;
}

uint32_t MAX31855::spiread32(void) {
  if (!initialized) return MAX31855_ERR_BIT;
  uint32_t d = 0;

  SPI.beginTransaction(spiSettings);

  digitalWrite(cs, LOW);
  // tCSS (CS fall to SCK rise) is specified as at least 100ns in the MAX31855 datasheet
  delayMicroseconds(1);

  // TODO: figure out why the one call to transfer didn't work and/or try doing transfer16()
  //SPI.transfer(&d, 4);
  d = SPI.transfer(0);
  d <<= 8;
  d |= SPI.transfer(0);
  d <<= 8;
  d |= SPI.transfer(0);
  d <<= 8;
  d |= SPI.transfer(0);
  
  digitalWrite(cs, HIGH);

  SPI.endTransaction();

  lastErr = d & 0x7;
  sinceLastRead = 0;
  return d;
}
