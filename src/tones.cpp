#include <Arduino.h>
#include "tones.h"
#include "pins.h"

const int tuneStartup[] PROGMEM = {NOTE_C5,8,NOTE_G4,8,-1};
const int tuneButtonPressed[] PROGMEM = {NOTE_F5,500,-1};
const int tuneInvalidPress[] PROGMEM = {NOTE_B2,10,-1};
const int tuneReflowDone[] PROGMEM = {NOTE_C5,4,NOTE_G4,8,NOTE_G4,8, NOTE_A4,4,NOTE_G4,4,0,4,NOTE_B4,4,NOTE_C5,4,-1};
const int tuneRemoveBoards[] PROGMEM = {NOTE_C5,4,NOTE_B4,4,NOTE_E4,2,-1};
const int tuneScreenshotBusy[] PROGMEM = {NOTE_C8,1000,-1};
const int tuneScreenshotDone[] PROGMEM = {NOTE_D2,10,NOTE_D4,10,NOTE_D6,6,-1};
const int tuneReflowBeep[] PROGMEM = {NOTE_C4,16,NOTE_B5,10,-1};

void playTones(const int *tune) {
  pinMode(PIN_BUZZER, OUTPUT);
  for (int i=0; tune[i] != -1; i+=2) {
    // Note durations: 4 = quarter note, 8 = eighth note, etc.:
    int duration = 1000/tune[i+1];
    tone(PIN_BUZZER, tune[i], duration);
    delay(duration * 1.1);
  }
  noTone(PIN_BUZZER);
}
