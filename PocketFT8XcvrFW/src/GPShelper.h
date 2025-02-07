#pragma once

#include <Adafruit_GPS.h>

#include "pins.h"

// what's the name of the hardware serial port?
#define GPSSerial Serial1

class GPShelper {
   public:
    // This public member records if we have ever obtained a successful fix with the data below
    bool validGPSdata = false;

    // These are the publicly accessible member variables whose values are valid only if we have obtained a fix
    unsigned year;
    unsigned month;
    unsigned day;
    unsigned hour;
    unsigned minute;
    unsigned second;
    unsigned milliseconds;
    double flat;
    double flng;
    uint32_t elapsedMillis;  // millis() elapsed runtime as of when we acquired GPS date/time

    // Publicly accessible methods
    GPShelper(unsigned gpsBaudRate);
    bool obtainGPSfix(unsigned timeoutSeconds, void (*gpsAcquiringFix)(unsigned));  // Returns true and assigns member var values if it obtains a GPS fix
    bool hasFix(void);

    // Our private implementation variables
   private:
};