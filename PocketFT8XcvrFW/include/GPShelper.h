#pragma once

#include <Adafruit_GPS.h>

#include "hwdefs.h"

// what's the name of the hardware serial port?
#define GPSSerial Serial1

/**
 * @brief GPShelper implemented as a Meyers singleton
 *
 * @note A reference to the one-and-only instance of the GPShelper object is available
 * through the static GPShelper::getInstance()
 */
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
    uint32_t elapsedMillis;  // millis() elapsed runtime as of when we acquired GPS date/time
    double flat;
    double flng;

    // Publicly accessible methods
    bool obtainGPSData(unsigned timeoutSeconds, void (*gpsAcquiringFix)(unsigned));  // Returns true and assigns member var values if it obtains a GPS fix
    volatile bool hasFix(void);

    // Delete the singleton's copy constructor and assignment operator
    GPShelper(const GPShelper&) = delete;
    GPShelper& operator=(const GPShelper&) = delete;
    static GPShelper& getInstance() {
        static GPShelper theInstance(9600);
        return theInstance;
    }

    // Our private implementation variables
   private:
    GPShelper(unsigned gpsBaudRate);  // Singleton's Constructor
};