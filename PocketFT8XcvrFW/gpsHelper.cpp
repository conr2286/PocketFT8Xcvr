#include <TimeLib.h>
#include <TinyGPS.h>

#include "pins.h"
#include "gpsHelper.h"
#include "DEBUG.h"

//Optional GPS connection
TinyGPS gps;
bool gpsInitialized = false;

#define TIMELYMANNER 60000  //Milliseconds to await GPS getting a fix

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
      if (gps.encode(Serial1.read())) {  //Process the gps message

        unsigned long age;
        int Year;
        byte Month, Day, Hour, Minute, Second;

        //Extract date/time and location from GPS message
        gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age);   //UTC
        gps.f_get_position(&flat, &flon, &age);

        //Only use recent GPS results
        if (age < 500) {

          // set the MCU Time to the GPS reading
          setTime(Hour, Minute, Second, Day, Month, Year);
          DPRINTF("GPS reports %2d/%2d/%2d %2d:%2d:%2d age=%u\n", Month, Day, Year, Hour, Minute, Second, age);

          //Now set the Teensy RTC to the GPS-derived time in the MCU
          Teensy3Clock.set(now());  
          return true;              //Success!
        }                           //if age ok

      }  //if gps.encode()

    }  //SerialGPS.available()

  }  //Timeout

  return false;  //Failure
}
