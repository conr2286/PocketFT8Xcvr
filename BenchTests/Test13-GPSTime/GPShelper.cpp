/**
 * @brief GPShelper decorates and conceals details of the GPS Library
 *
 *
 * FEATURES
 *  + Conceals the GPS Library interface from Pocket FT8, simplifying its replacement.
 *  + Waits for the GPS device to acquire a fix and produce quality readings
 *  + Optional notification via a callback function during a long delay
 *  + Times-out if GPS is missing or unable to obtain a fix
 *
 *
**/





#include <TimeLib.h>

#include "pins.h"
#include "GPShelper.h"
#include "DEBUG.h"

static Adafruit_GPS gps(&Serial1);

/**
  ** GPShelper Constructor
  **
  ** @param baudRate GPS serial connection baud rate
  **
  **/
GPShelper::GPShelper(unsigned gpsBaudRate) {

  Serial.begin(9600);
  DTRACE();
  delay(10);

  gps.begin(9600);                                //Set Serial1 baud rate to GPS device
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  //We are only interested in RMC messages from GPS
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);     //10 Hz updates improve time resolution
  delay(1000);

}  //GPShelper

/**
 *  @brief Loops to obtain date, time and location from GPS 
 *
 *  @param timeoutSeconds Number of seconds to wait for GPS to acquire a fix
 *  @param gpsAcquiringFix() Callback function (or NULL) notified once/second during acquisition
 *
 *  @return true if a GPS fix and all member variables are valid, false if timeout
 *
 * The purpose of obtainGPSfix() is to loop processing GPS messages until we've
 * obtained valid UTC date, time *and* our location.  Not all GPS messages contain
 * all the data we seek, so the contribution here is to ensure we get all three.
 *
 * If we return success (true), then date, time and location have all been updated
 * in the GPShelper public member variables, and are available for your use.
 *
 * If you don't want to be notified while GPShelper acquires a fix, then supply
 * a NULL pointer for the gpsAcquiringFix parameter.
 *
 * Design Notes:  While the time accuracy is on the order of <100 ms, the result
 * can be somewhat improved by increasing the GPS serial connection's baud rate.
 * However, doing so causes the GPS to remember the new baud rate and requires the
 * code to deal with both the default baud rate (9600) for a cold start as well as
 * the increased baud rate for a warm start.  It doesn't seem worth the trouble.
 *
 * The 10Hz update rate proved important to obtaining accurate time readings.
 *
**/
bool GPShelper::obtainGPSfix(unsigned timeoutSeconds, void (*gpsAcquiringFix)()) {

  //A successful result requires all three flags to become true
  bool gotDate = false;
  bool gotTime = false;
  bool gotLoc = false;

  //Starting time for timeout loop
  unsigned long t0 = millis();
  unsigned long t1 = t0;  //Triggers callback function

  //This is the GPS time-out loop
  while ((millis() - t0) <= timeoutSeconds * 1000) {

    //gpsAcquiringFix() callback is notified once/second
    if ((millis() - t1) >= 1000) {
      t1 = millis();         //Update second monitor
      if (gpsAcquiringFix!=NULL) (*gpsAcquiringFix)();  //Notify callback function if provided
    }

    //Process message byte(s), if any, from GPS
    if (gps.read()) {

      //We have message bytes, do we have a complete NMEA message?
      if (gps.newNMEAreceived()) {  

        //Yes, parse the complete NMEA message, checking for errors
        if (gps.parse(gps.lastNMEA())) {

          //Message is good, retrieve GPS supplied location if it appears valid
          if (gps.fix && gps.fixquality == 0) {
            flat = gps.latitudeDegrees;
            flng = gps.longitudeDegrees;
            DPRINTF("GPS reports lat/lon = %f %f\n", flat, flng);
            gotLoc = true;
          }

          //Retrieve date/time if it seems accurate and recent.  Warning:  milliseconds may exceed 999
          //if the GPS reported time is stale and nearly wrapping 1 second.
          if (gps.fix && gps.secondsSinceTime() < 0.500) {
            hour = gps.hour;
            minute = gps.minute;
            second = gps.seconds;
            milliseconds = gps.milliseconds + gps.secondsSinceTime() * 1000.0;
            year = gps.year;
            month = gps.month;
            day = gps.day;
            DPRINTF("GPS time = %02d:%02d:%02d.%04d UTC, age=%f secs\n", hour, minute, second, milliseconds, gps.secondsSinceTime());
            gotTime = gotDate = true;
          }

          //We are finished when/if we've acquired date, time and location
          if (gotDate && gotTime && gotLoc) {
            return true;  //Success
          }

        }  // successful parse of message

      }  // received complete message

    }  // read() one byte

  }  //Timeout loop

  return false;  //Failure
}
