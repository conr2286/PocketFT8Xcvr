/**
 *
 * TimeExplorer --- Loops printing the UTC time to Serial
 *
 * Hardware
 *  Teensy 4.1
 *
 * Description
 *  Teensy 4.1 maintains the Time of Day (ToD) in its own on-board, battery-backed
 *  RTC.  The clock's time is set by the loader.  This explorer aims to reveal
 *  the accuracy of the Teensy's RTC setting.
 *
 * Authors
 *  JimC
**/

#include <TimeLib.h>

int h, m, s, sOld;

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void setup() {

  //Initialization
  Serial.begin(9600);
  Serial.printf("Starting...\n");
  setSyncProvider(getTeensy3Time);
  sOld = -1;

  //Add fudge-factor to correct Teensy loader's ~1 second tardiness
  adjustTime(1);
}

void loop() {

  //Get teensy time
  Teensy3Clock.get();
  h = hour();
  m = minute();
  s = second();

  //Print time when second changes
  if (s != sOld) {
    Serial.printf("%2i:%2i:%2i\n", h, m, s);
    sOld = s;
  }
}
