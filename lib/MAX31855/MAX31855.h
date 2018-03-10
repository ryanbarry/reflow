#ifndef MAX31855_H
#define MAX31855_H

#include "Arduino.h"
#include <SPI.h>

#define MAX31855_SPISPEED 4000000
#define MAX31855_ERR_BIT 0x00010000
#define MAX31855_NO_ERR 0
#define MAX31855_ERR_OC  0b001
#define MAX31855_ERR_SCG 0b010
#define MAX31855_ERR_SCV 0b100

class MAX31855 {
 public:
  MAX31855(int8_t _ck, int8_t _cs, int8_t _miso);

  void begin(void);
  int32_t read16thDegInternal(void);
  int32_t readQtrDegCelsius(void);
  uint8_t readError(void);

 private:
  boolean initialized;
  uint8_t lastErr;
  SPISettings spiSettings;

  int8_t ck, miso, cs;
  int32_t v;
  elapsedMillis sinceLastRead;
  uint32_t spiread32(void);
};

#endif
