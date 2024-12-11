#include <TimeLib.h>
#include <TinyGPSPlus.h>

#include "pins.h"
#include "GPShelper.h"
#include "DEBUG.h"

/**
 *  @brief Loops to obtain date, time and location from GPS 
 *
 *  @return true if a GPS fix and all member variables are valid, false if timeout
 *
 * The purpose of the obtainGPSfix() is to loop processing GPS messages until we've
 * obtained valid UTC date, time and our location.  Not all GPS messages contain
 * all the data we seek, so the contribution here is to ensure we get it all.
 *
 * If we return success (true), then date, time and location have all been updated
 * with the most recent GPS result.
 *
**/
bool GPShelper::obtainGPSfix() {

  //A successful result requires all three of these flags to become true
  bool gotDate = false;  
  bool gotTime = false;   
  bool gotLoc = false;

  //Starting time
  unsigned long t0 = millis();

  //This is the GPS time-out loop
  while ((millis() - t0) <= timeoutSeconds) {

    //This inner loop processes incoming message bytes from the GPS
    while (SerialGPS.available()) {

      //Read and process a received GPS message byte
      if (gps.encode(SerialGPS.read())) {  //Returns true if we've received a complete message

        //Retrieve GPS supplied date if valid
        if (!gotDate && gps.date.isValid()) {

          year = gps.date.year();
          month = gps.date.month();
          day = gps.date.day();

          DPRINTF("GPS reports date = %02d/%02d/%02d\n", month, day, year);
          gotDate = true;
        }

        //Retrieve GPS supplied time if valid
        if (!gotTime && gps.time.isValid()) {
          hour = gps.time.hour();
          minute = gps.time.minute();
          second = gps.time.second();
          DPRINTF("GPS reports time = %02d:%02d:%02d\n", hour, minute, second);
          gotTime = true;
        }

        //Retrieve GPS supplied location if valid
        if (!gotLoc && gps.location.isValid()) {
          flat = gps.location.lat();
          flng = gps.location.lng();
          DPRINTF("GPS reports lat/lon = %f %f\n", flat, flng);
          gotLoc = true;
        }

        // //Update MCU and RTC date and time if GPS has supplied both
        // if (gotDate && gotTime) {

        //   //First, set the MCU Time to the GPS reading
        //   setTime(hr, mi, sc, dy, mo, yr);
        //   DPRINTF("setTime %2d/%2d/%2d %2d:%2d:%2d\n", mo, dy, yr, hr, mi, sc);

        //   //Now set the Teensy RTC to the GPS-derived time in the MCU
        //   Teensy3Clock.set(now());
        // }

        //We are finished if we've acquired date, time and loc
        if (gotDate && gotTime && gotLoc) {
          return true;  //Success
        }

      }  //if gps.encode()

    }  //SerialGPS.available()

  }  //Timeout loop

  return false;  //Failure
}
