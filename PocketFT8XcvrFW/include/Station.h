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
    const char* getMyName(void) { return myName.c_str(); }  // Operator's name, not callsign
    unsigned getFrequency(void) { return frequency; }       // kHz

    // The setters
    void setCallsign(String s) { callsign = s; }
    void setLocator(String s) { locator = s; }
    void setRig(String s) { rig = s; }
    void setMyName(String s) { myName = s; }
    void setFrequency(unsigned kHz) { frequency = kHz; }

   private:
    // The station model data
    String callsign;
    String locator;
    String rig;
    String myName;
    unsigned frequency;  // kHz
};