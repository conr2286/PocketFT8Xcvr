#include <TimeLib.h>
#include <TinyGPS.h>

#include "pins.h"
#include "gpsHelper.h"
#include "DEBUG.h"

//Optional GPS connection
TinyGPS gps;
bool gpsInitialized = false;

#define TIMELYMANNER 60000  //Milliscs to await GPS getting a fix

/**
 *  Sync MCU and RTC time with GPS if the GPS is available with a fix
 *
 *  @return true if GPS is communicating and has a fix, false otherwise
 *
 * syncGPSTime() will time-out if GPS can't get a lock in a timely manner
 *
**/
bool syncGPSTime() {

  //Initialize serial port connection if not yet accessed
  if (!gpsInitialized) {
    Serial1.begin(9600);
    gpsInitialized = true;
  }

  unsigned long t0 = millis();
  float flat, flon;


  //This is the GPS time-out loop
  while ((millis() - t0) <= TIMELYMANNER) {

    //This loop processes the data from a GPS message (if any)
    while (Serial1.available()) {

      //Read and encode the received GPS message
      char c = Serial1.read();
      //Serial.write(c);
      if (gps.encode(c)) {  //Process the gps message

        unsigned long age;
        int yr;
        byte mo, dy, hr, mi, sc, hu;

        //Extract date/time and location from GPS message
        gps.crack_datetime(&yr, &mo, &dy, &hr, &mi, &sc, &hu, &age);  //UTC
        DPRINTF("GPS reports %2d/%2d/%2d %2d:%2d:%2d:%2d age=%u\n", mo, dy, yr, hr, mi, sc, hu, age);
        gps.f_get_position(&flat, &flon, &age);

        //Only use recent GPS results
        if (age < 500) {

          //DPRINTF("year()=%u\n",year());

          //Repair the date if GPS didn't supply it
          if (yr == 2000) {
            yr = year();  //Use whatever Teensy reports
            mo = month();
            dy = day();
          }

          // set the MCU Time to the GPS reading
          setTime(hr, mi, sc, dy, mo, yr);
          DPRINTF("setTime %2d/%2d/%2d %2d:%2d:%2d:%2d\n", mo, dy, yr, hr, mi, sc, hu);


          //Now set the Teensy RTC to the GPS-derived time in the MCU
          Teensy3Clock.set(now());

          //Calculate time required to obtain a fix
          DPRINTF("GPS time to fix:  %lu ms\n", millis()-t0);
          return true;  //Success!
        }               //if age ok

      }  //if gps.encode()

    }  //SerialGPS.available()

  }  //Timeout

  return false;  //Failure
}
