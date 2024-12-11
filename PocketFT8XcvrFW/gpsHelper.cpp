#include <TimeLib.h>
#include <TinyGPSPlus.h>

#include "pins.h"
#include "gpsHelper.h"
#include "DEBUG.h"


//Optional GPS connection
TinyGPSPlus gps;
bool gpsInitialized = false;

#define TIMELYMANNER 10000  //Milliscs to await GPS getting a fix

/**
 *  Sync MCU and RTC time with GPS if the GPS is available with a fix
 *
 *  @return true if GPS is communicating and has a fix, false otherwise
 *
 * syncGPSTime() will time-out if GPS can't get a lock in a timely manner
 *
**/
bool syncGPSTime() {


  DTRACE();
  bool gotDate = false;
  bool gotTime = false;
  bool gotLoc = false;

  //Initialize serial port connection if not yet accessed
  if (!gpsInitialized) {
    SerialGPS.begin(9600);
    gpsInitialized = true;
  }

  unsigned long t0 = millis();
  float flat, flon;


  //This is the GPS time-out loop
  while ((millis() - t0) <= TIMELYMANNER) {
    //DTRACE();

    //This loop processes incoming message bytes from the GPS
    while (SerialGPS.available()) {
      //DTRACE();

      //Read and process a received GPS message byte
      if (gps.encode(SerialGPS.read())) {  //Returns true if we've received a complete message
        DTRACE();

        unsigned long age;
        int yr;
        byte mo, dy;
        byte hr, mi, sc;

        //Retrieve GPS supplied date if valid
        if (!gotDate && gps.date.isValid()) {

          yr = gps.date.year();
          mo = gps.date.month();
          dy = gps.date.day();

          DPRINTF("GPS reports date = %02d/%02d/%02d\n", mo, dy, yr);
          gotDate = true;
        }

        //Retrieve GPS supplied time if valid
        if (!gotTime && gps.time.isValid()) {
          hr = gps.time.hour();
          mi = gps.time.minute();
          sc = gps.time.second();
          DPRINTF("GPS reports time = %02d:%02d:%02d\n", hr, mi, sc);
          gotTime = true;
        }

        //Retrieve GPS supplied location if valid
        if (!gotLoc && gps.location.isValid()) {
          flat = gps.location.lat();
          flon = gps.location.lng();
          DPRINTF("GPS reports lat/lon = %f %f\n", flat, flon);
          gotLoc = true;
        }

        //Update MCU and RTC date and time if GPS has supplied both
        if (gotDate && gotTime) {

          //First, set the MCU Time to the GPS reading
          setTime(hr, mi, sc, dy, mo, yr);
          DPRINTF("setTime %2d/%2d/%2d %2d:%2d:%2d\n", mo, dy, yr, hr, mi, sc);

          //Now set the Teensy RTC to the GPS-derived time in the MCU
          Teensy3Clock.set(now());
        }

        //We are finished if we've acquired date, time and loc
        if (gotDate && gotTime && gotLoc) {
          DTRACE();
          return true;  //Success
        }

      }  //if gps.encode()

    }  //SerialGPS.available()

  }  //Timeout loop

  return false;  //Failure
}
