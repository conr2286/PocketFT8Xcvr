/**
 * @brief Station model
 */
#pragma once
#include <Arduino.h>

class Station {
   public:
    // The getters
    const char* getCallsign(void) { return callsign.c_str(); }
    const char* getLocator(void) { return locator.c_str(); }
    const char* getRig(void) { return rig.c_str(); }
    unsigned getFrequency(void) { return frequency; }  // kHz

    // The setters
    void setCallsign(String s) { callsign = s; }
    void setLocator(String s) { locator = s; }
    void setRig(String s) { rig = s; }
    void setFrequency(unsigned kHz) { frequency = kHz; }

   private:
    // The station model data
    String callsign;
    String locator;
    String rig;
    unsigned frequency;  // kHz
};