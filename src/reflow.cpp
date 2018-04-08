#include "reflow.h"

#include <Arduino.h>
#include "display.h"

typedef enum {
  STATE_WAIT_UNTIL_ABOVE_C,
  STATE_WAIT_UNTIL_BELOW_C,
  STATE_ELEMENTS_ON,
  STATE_RAMP,
  STATE_WAIT_FOR_S
} REFLOW_STATES;

void reflowStatus(const char*);
void drawReflowScreen(void);

void reflow(void) {
/*  // max deviation 20C
  // max temperature 260C
  
  // close door
  // cooling fan off
  // stop timer
  // display "Waiting for oven to cool to 50C"
  // wait-until-below 50C
  reflowStatus("Waiting for oven to cool to 50C");
  waitUntilBelowC(50);

  //NOTE: start of cycle
  // display "Phase: Pre-soak"
  // maximum duty cycle override 100/100/60
  // element-duty-cycle 100/100/60
  // wait-until-above 75C
  // maximum duty cycle back to 100/75/60
  reflowStatus("Phase: Pre-soak");
  clampDutyCycle(100,100,60);
  elementDutyCycle(100,100,60);
  waitUntilAboveC(75);
  clampDutyCycle(100,75,60);

  // start timer at 15s
  // set-bias 100/80/60
  // ramp-to 150C in 70s
  startTimer(15);
  setElementBias(100,80,60);
  rampTo(150, 70);

  //NOTE: start of soak
  // display "Phase: Soak"
  // set-bias 100/50/20
  // ramp-to 200C in 140s
  reflowStatus("Phase: Soak");
  setElementBias(100,50,20);
  rampTo(200, 140);

  //NOTE: start reflowing
  // display "Phase: Reflow"
  // set-bias 100/80/60
  // ramp-to 250 in 60s
  reflowStatus("Phase: Reflow");
  setElementBias(100,80,60);
  rampTo(250, 60);
  
  // display "Phase: Reflow (hold)"
  // element-duty-cycle 20/20/0
  // wait 30s
  // stop-timer
  // element-duty-cycle 0/0/0 (off)
  reflowStatus("Phase: Reflow (hold)");
  elementDutyCycle(20,20,0);
  waitSeconds(30);
  stopTimer();
  elementDutyCycle(0,0,0);
  
  //NOTE: done reflowing, cool down now
  // display "Phase: Cooling"
  // open door
  // wait 10s
  // cooling fan on
  // wait-until-below 100C
  reflowStatus("Phase: Cooling");
  waitUntilBelowC(100);
  
  //NOTE: should be safe-ish to grab stuff now
  // display "Boards can be removed"
  // play-tune
  // wait-until-below 50C
  reflowStatus("Boards can be removed");
  waitUntilBelowC(50);
  
  // display "Reflow Complete!"
  // cooling fan off
  // close door
  reflowStatus("Reflow Complete!");
*/
}

void reflowStatus(const char *s) {
  tft.setTextSize(1);
  tft.setCursor(10, 229);
  tft.setFont(&HEADER_FONT);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(s);
}

void drawReflowScreen(void) {
  tft.fillRect(0, 28, 320, 240-28, ILI9341_WHITE); // clear everything below header
  tft.fillRect(8, 216, 304, 2, ILI9341_GREEN);
  const int graphBottom=205, graphTop=31,
    xTickSeparation=10, yTickSeparation=15,
    graphWidth=280, graphHeight=175,
    graphLeft=30, graphRight=graphLeft+graphWidth;
  //TODO: do i need the "-1" after graphHeight/Width? (and "+1" on x-coord of right side Y-axis?)
  tft.drawFastVLine(30, graphTop, graphHeight-1, ILI9341_BLACK); // left side Y-axis
  tft.drawFastVLine(graphRight+1, graphTop, graphHeight-1, ILI9341_BLACK); // right side Y-axis
  tft.drawFastHLine(30, graphTop, graphWidth-1, ILI9341_BLACK); // top X-axis
  tft.drawFastHLine(30, graphBottom, graphWidth-1, ILI9341_BLACK); // bottom X-axis
  tft.setFont();
  tft.setTextSize(1);
  for(int i=0; i < (graphHeight/yTickSeparation+1); i++) {
    tft.drawFastHLine(25, graphBottom-(i*yTickSeparation), 7, ILI9341_BLACK); // y-axis tick marks
    // draw labels next to every other tick mark
    if (i%2 == 0) {
      if (i == 0) {
        tft.setCursor(6+6, graphBottom-(i*yTickSeparation) - 3);
      } else {
        tft.setCursor(6, graphBottom-(i*yTickSeparation) - 3);
      }
      tft.print(75+i/2*35);
    }
  }

  for(int i=1; i < (graphWidth/xTickSeparation - 1); i++) {
    if(i%3 == 0) {
      tft.drawFastVLine(graphLeft+i*xTickSeparation, graphBottom, 3, ILI9341_BLACK);
      tft.setCursor(graphLeft+(i-1)*xTickSeparation+2, graphBottom+4);
      tft.print(i*10); // draw labels every 3rd tick
    } else {
      tft.drawFastVLine(graphLeft+i*xTickSeparation, graphBottom, 2, ILI9341_BLACK);
    }
  }
}
