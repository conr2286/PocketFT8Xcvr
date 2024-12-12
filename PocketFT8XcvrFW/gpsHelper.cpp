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
 * obtained valid UTC date, time *and* our location.  Not all GPS messages contain
 * all the data we seek, so the contribution here is to ensure we get all three.
 *
 * If we return success (true), then date, time and location have all been updated
 * with the current GPS result.
 *
**/
bool GPShelper::obtainGPSfix(unsigned timeoutSeconds) {

  //A successful result requires all three flags to become true
  bool gotDate = false;
  bool gotTime = false;
  bool gotLoc = false;

  //Starting time for timeout loop
  unsigned long t0 = millis();

  //This is the GPS time-out loop
  while ((millis() - t0) <= timeoutSeconds * 1000) {

    //This inner loop processes incoming message bytes from the GPS
    while (SerialGPS.available()) {
      //DTRACE();

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

        //We are finished when/if we've acquired date, time and loc
        if (gotDate && gotTime && gotLoc) {
          return true;  //Success
        }

      }  //if gps.encode()

    }  //SerialGPS.available()

  }  //Timeout loop

  return false;  //Failure
}
