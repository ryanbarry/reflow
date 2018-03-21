#ifndef _XPT2046_CALIBRATED_H
#define _XPT2046_CALIBRATED_H

#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <stdint.h>

class XPT2046_Calibrated : public XPT2046_Touchscreen {
public:
  XPT2046_Calibrated(uint8_t _cs, uint8_t _irq = 255) : XPT2046_Touchscreen(_cs, _irq) {}

  void setRotation(uint8_t n) {
    rotation = n;
    XPT2046_Touchscreen::setRotation(n);
  }

/* run 1 (no rotation set)
x  0.0891  0.000137 19528
y -0.00179 0.0669   19548

   run 2 (no rotation set)
x 0.0896   -0.000709 2409
y 0.000157  0.0652   2429

   run 3 (rotation set to 3)
x -0.0886   -0.00135 333
y -0.000382 -0.0660  255

   run 4 (rotation set to 3)
x -0.0882  -0.000174 329
y 0.000238 -0.0668   257

   run 5 (rotation set to 3)
x -0.0899    0.000855 330
y -0.000571 -0.06510  256 
 */

  void runCalibration(Adafruit_ILI9341&);
  void calibrate(float ax, float bx, int16_t dx, float ay, float by, int16_t dy, uint16_t dwidth, uint16_t dheight) {
    alpha_x = ax;
    beta_x = bx;
    delta_x = dx;
    alpha_y = ay;
    beta_y = by;
    delta_y = dy;
    displ_width = dwidth;
    displ_height = dheight;
  }

  void calibRead(uint16_t *x, uint16_t *y, uint8_t *z);

private:
  uint8_t rotation;
  int16_t delta_x, delta_y, displ_width, displ_height;
  float alpha_x, beta_x, alpha_y, beta_y;
};

#endif
