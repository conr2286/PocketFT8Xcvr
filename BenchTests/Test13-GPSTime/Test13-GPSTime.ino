/*
 * NAME
 *  Test13-GPSTime - Exercises the GPS module to precisely get/set UTC time and maidenhead locator
 *
 * EXERCISED
 *  Adafruit_GPS Library
 *  Teensy Serial1
 *  GPS device connected to Serial1
 *  Maidenhead locator
 *
 * NOTES
 *  Pocket FT8 requires accurate (sub-second) timing to achieve successful decodes.  In addition,
 *  its portable SOTA/POTA mission requires knowledge of the station's maidenhead grid square.
 *
 *  There are many GPS devices and libraries to choose from.  Adafruit's "Ultimate GPS" offers
 *  battery-backed "warm start" reducing the time to acquire a fix from minutes to seconds.
 *  The Adafruit library offers a 10 Hz (100 ms period) update rate and discloses the elapsed
 *  time between when the library obtained the GPS time until when the library supplied that
 *  report to the application.  These features enable sub-second accuracy for FT8.
 *
 *  If all goes well, this test will acquire a GPS fix (which may require minutes for a new
 *  device), and report the current location (latitude, longitude and maidenhead grid square),
 *  and the current UTC date and time over the USB serial connection to the host computer's IDE.
 *  If timestamps are enabled in the Arduino IDE Serial Monitor, the local time can be compared
 *  to the UTC time reports.  These should be closely aligned, possibly ~100 ms or even less.
 *
 *  During development, tests run at high (>9600) baud rates provided slightly improved time
 *  accuracy at the cost of awkward GPS configuration as the device "remembered" its baud rate
 *  configuration through power cycles which meant the software had to accommodate different
 *  speeds for a new (cold start) vs. "experienced" (warm start) device.
 *
 * LIMITATIONS
 *  The PPS signal is not exercised, though it is used in the firmware when available.  Sadly,
 *  the V2.00 hardware requires a patch wire to connect the PPS signal from the GPS to Teensy.
 *
**/
#include <TimeLib.h>
#include "DEBUG.h" 
#include "GPShelper.h"
#include "maidenhead.h"

char Locator[20];

//Build the GPSHelper decorating gps lib with Pocket FT8 functionality
GPShelper gps(9600);

void gpsAcquiringFix() {
  DPRINTF("Acquiring gps fix\n");
}

void setup() {

  //Get the USB serial port running before something else goes wrong
  Serial.begin(9600);
  delay(100);
  DTRACE();

  //Sync MCU and RTC time with GPS if it's working and can get a timely fix
  if (gps.obtainGPSfix(60,gpsAcquiringFix)) {

    //Set the MCU time to the GPS result
    setTime(gps.hour, gps.minute, gps.second, gps.day, gps.month, gps.year);
    DPRINTF("GPS time = %02d/%02d/%02d %02d:%02d:%02d.%02d UTC\n", gps.month, gps.day, gps.year, gps.hour, gps.minute, gps.second,gps.milliseconds);

    //Now set the Teensy RTC to the GPS-derived time in the MCU
    Teensy3Clock.set(now());

    //Use the GPS-derived locator 
    strlcpy(Locator, get_mh(gps.flat, gps.flng, 4), sizeof(Locator));
    DPRINTF("GPS derived Locator = %s\n", Locator);
  } else {
    DPRINTF("Unable to obtain GPS data\n");
  }

  DPRINTF("Finished\n");
}

bool initialized=false;
unsigned long t0;
  void loop() {

    //Note millis when loop begins
    if (!initialized) {
      initialized=true;
      t0 = millis();
    }
    
    //Print timing messages once/second in loop
    Serial.printf("TimeLib %02d:%02d:%02d, elapsed=%lu\n", hour(), minute(), second(),millis()-t0);

    //Wait a second
    delay(1000);
  }
