#include "XPT2046_Calibrated.h"

/* calibration code from http://www.ars-informatica.ca/eclectic/xpt2046-touch-screen-three-point-calibration/ */

void XPT2046_Calibrated::calibRead(uint16_t *x, uint16_t *y, uint8_t *z) {
  uint16_t xraw, yraw;
  XPT2046_Touchscreen::readData(&xraw, &yraw, z);

  *x = alpha_x * xraw + beta_x * yraw + delta_x;
  *y = alpha_y * xraw + beta_y * yraw + delta_y;

  if(rotation != 0) {
    uint16_t rot_x, rot_y; // temp vars

    switch (rotation) {
    case 1:
      rot_x = *y;
      rot_y = displ_height - *x;
      break;
    case 2:
      rot_x = displ_width - *x;
      rot_y = displ_height - *y;
      break;
    case 3:
      rot_x = displ_width - *y;
      rot_y = *x;
      break;
    default:
      break;
    }

    *x = rot_x;
    *y = rot_y;
  }
}

/*  Custom ftoa() function: converts float to string

Specifically: converts a float value to alphanumeric values in a zero-terminated char array,
similar to itoa, and stores the result in the array specified by str.

Usage: ftoa (float value, char * str, uint8_t precision);

str should be an array long enough to contain any possible value. Precision indicates the number of
significant figures in the result, not the number of decimals.

In this implementation, ftoa() handles floats with an integer component up to 4,294,967,295 in size;
feel free to adapt to a uint64_t for an integer component of up to 18,446,744,073,709,551,615. Since
I don't need it, and it's faster, and uint64_t implementation in Arduino at times doesn't work, I've
kept it as a uint_32 version. Also, if you need more than 10 significant figures in precision
(*really???*) then you'll need to increase character array s1 accordingly - allow one space for
possible decimals, and one for zero-terminating the array */

void ftoa(float f, char *str, uint8_t precision) {
  uint8_t i, j, divisor = 1;
  int8_t log_f;
  int32_t int_digits = (int)f;             //store the integer digits
  float decimals;
  char s1[12];

  memset(s1, 0, sizeof(s1));

  if (f < 0) {                             //if a negative number 
    str[0] = '-';                          //start the char array with '-'
    f = abs(f);                            //store its positive absolute value
  }
  log_f = ceil(log10(f));                  //get number of digits before the decimal
  if (log_f > 0) {                         //log value > 0 indicates a number > 1
    if (log_f == precision) {              //if number of digits = significant figures
      f += 0.5;                            //add 0.5 to round up decimals >= 0.5
      itoa(f, s1, 10);                     //itoa converts the number to a char array
      strcat(str, s1);                     //add to the number string
    }
    else if ((log_f - precision) > 0) {    //if more integer digits than significant digits
      i = log_f - precision;               //count digits to discard
      divisor = 10;
      for (j = 0; j < i; j++) divisor *= 10;    //divisor isolates our desired integer digits 
      f /= divisor;                             //divide
      f += 0.5;                            //round when converting to int
      int_digits = (int)f;
      int_digits *= divisor;               //and multiply back to the adjusted value
      itoa(int_digits, s1, 10);
      strcat(str, s1);
    }
    else {                                 //if more precision specified than integer digits,
      itoa(int_digits, s1, 10);            //convert
      strcat(str, s1);                     //and append
    }
  }

  else {                                   //decimal fractions between 0 and 1: leading 0
    s1[0] = '0';
    strcat(str, s1);
  }

  if (log_f < precision) {                 //if precision exceeds number of integer digits,
    decimals = f - (int)f;                 //get decimal value as float
    strcat(str, ".");                      //append decimal point to char array

    i = precision - log_f;                 //number of decimals to read
    for (j = 0; j < i; j++) {              //for each,
      decimals *= 10;                      //multiply decimals by 10
      if (j == (i-1)) decimals += 0.5;     //and if it's the last, add 0.5 to round it
      itoa((int)decimals, s1, 10);         //convert as integer to character array
      strcat(str, s1);                     //append to string
      decimals -= (int)decimals;           //and remove, moving to the next
    }
  }
}

void drawCalibrationPoint(Adafruit_ILI9341 &tft, uint16_t calX, uint16_t calY) {
  tft.drawCircle(calX, calY, 6, ILI9341_WHITE);
  tft.drawFastHLine(calX-4, calY, 9, ILI9341_WHITE);
  tft.drawFastVLine(calX, calY-4, 9, ILI9341_WHITE);
}

void XPT2046_Calibrated::runCalibration(Adafruit_ILI9341 &tft) {
  uint16_t calA[] = {10, 15};             //calibration points must be independent, i.e. not line up
  uint16_t calB[] = {280, 80};
  uint16_t calC[] = {170, 200};

  uint16_t calA_raw[] = {0, 0};           //store raw touch data for calibration points
  uint16_t calB_raw[] = {0, 0};
  uint16_t calC_raw[] = {0, 0};

  uint8_t calCount = 0;                   //track which point has been activated

  char s[30];                             //character arrays for text strings

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);

  tft.setTextSize(1);                   //these should be self-explanatory
  tft.setCursor(20,43);
  tft.print("touch stylus to target centre");

  tft.setTextSize(2);
  tft.setCursor(20,25);
  tft.print("Calibrate Touch:");
  tft.setCursor(35,63);
  tft.print("X:");
  tft.setCursor(35,82);
  tft.print("Y:");
  tft.setCursor(35,101);
  tft.print("Z:");

  drawCalibrationPoint(tft, calA[0], calA[1]);

  while(1) {
    if (touched()) {                   //check for screen touch
      tft.fillRect(59, 63, 48, 54, ILI9341_BLACK);
      //black out X, Y, Z values
      TS_Point p = getPoint();         //get touch data
      memset(s, 0, sizeof(s));  
      itoa(p.x, s, 10);                   //convert X data to char array
      tft.setCursor(59,63);
      tft.print(s);                       //and display
      memset(s, 0, sizeof(s));  
      itoa(p.y, s, 10);                   //same for Y value
      tft.setCursor(59,82);
      tft.print(s);
      memset(s, 0, sizeof(s));  
      itoa(p.z, s, 10);                   //and Z (pressure)
      tft.setCursor(59,101);
      tft.print(s);

      if (calCount == 0) {                //first calibration point?
        calA_raw[0] = p.x;                //store values
        calA_raw[1] = p.y; 

        tft.fillRect(calA[0]-6, calA[1]-6, 13, 13, ILI9341_BLACK);
        //black out calibration point
        delay(500);                       //give user time to take stylus off screen so as not to
        //read same values into next calibration point
        drawCalibrationPoint(tft, calB[0], calB[1]);
        //and display the next calibration point
      }
      else if (calCount == 1) {           //do the same for the second calibration point
        calB_raw[0] = p.x;
        calB_raw[1] = p.y;

        tft.fillRect(calB[0]-6, calB[1]-6, 13, 13, ILI9341_BLACK);
        delay(500);
        drawCalibrationPoint(tft, calC[0], calC[1]);
      }
      else if (calCount == 2) {           //and third
        calC_raw[0] = p.x;
        calC_raw[1] = p.y;

        tft.fillRect(calC[0]-6, calC[1]-6, 13, 13, ILI9341_BLACK);

        tft.setTextSize(1);               //get ready to display calibration parameters
        tft.setCursor(20,130);
        tft.print("Parameters:");
        tft.setCursor(15,155);
        tft.print("X:");
        tft.setCursor(110,155);
        tft.print("Y:");

        //calculate calibration parameters

        int32_t delta = (calA_raw[0]-calC_raw[0])*(calB_raw[1]-calC_raw[1]) -
          (calB_raw[0]-calC_raw[0])*(calA_raw[1]-calC_raw[1]); 
        float alpha_x = (float)((calA[0]-calC[0])*(calB_raw[1]-calC_raw[1]) -
                                (calB[0]-calC[0])*(calA_raw[1]-calC_raw[1])) / delta; 
        float beta_x = (float)((calA_raw[0]-calC_raw[0])*(calB[0]-calC[0]) -
                               (calB_raw[0]-calC_raw[0])*(calA[0]-calC[0])) / delta;
        uint16_t delta_x = ((uint64_t)calA[0]*(calB_raw[0]*calC_raw[1]-calC_raw[0]*calB_raw[1]) -
                            (uint64_t)calB[0]*(calA_raw[0]*calC_raw[1]-calC_raw[0]*calA_raw[1]) + 
                            (uint64_t)calC[0]*(calA_raw[0]*calB_raw[1]-calB_raw[0]*calA_raw[1])) / delta;
        float alpha_y = (float)((calA[1]-calC[1])*(calB_raw[1]-calC_raw[1]) -
                                (calB[1]-calC[1])*(calA_raw[1]-calC_raw[1])) / delta; 
        float beta_y = (float)((calA_raw[0]-calC_raw[0])*(calB[1]-calC[1]) -
                               (calB_raw[0]-calC_raw[0])*(calA[1]-calC[1])) / delta;
        uint16_t delta_y = ((uint64_t)calA[1]*(calB_raw[0]*calC_raw[1]-calC_raw[0]*calB_raw[1]) -
                            (uint64_t)calB[1]*(calA_raw[0]*calC_raw[1]-calC_raw[0]*calA_raw[1]) +
                            (uint64_t)calC[1]*(calA_raw[0]*calB_raw[1]-calB_raw[0]*calA_raw[1])) / delta;

        memset(s, 0, sizeof(s));
        ftoa(alpha_x, s, 3);              //convert first parameter, a float value, to char array
        tft.setCursor(39,155);
        tft.print(s);                     //and display
        memset(s, 0, sizeof(s));  
        ftoa(beta_x, s, 3);
        tft.setCursor(39,173);
        tft.print(s);
        memset(s, 0, sizeof(s));  
        itoa(delta_x, s, 10);             //etc.
        tft.setCursor(39,191);
        tft.print(s);
        memset(s, 0, sizeof(s));
        ftoa(alpha_y, s, 3);
        tft.setCursor(134,155);
        tft.print(s);
        memset(s, 0, sizeof(s));
        ftoa(beta_y, s, 3);
        tft.setCursor(134,173);
        tft.print(s);
        memset(s, 0, sizeof(s));  
        itoa(delta_y, s, 10);
        tft.setCursor(134,191);
        tft.print(s);  

        tft.setCursor(200, 20);
        tft.print("[ Restart ]");
      }

      calCount++;
    }

    if (calCount == 4) {                  //finally, if another touch is detected
      tft.fillRect(0, 130, 240, 190, ILI9341_BLACK);
      //clear parameters from screen
      drawCalibrationPoint(tft, calA[0], calA[1]); //and start over
      calCount = 0;
    }

    delay(200);
  }
}
