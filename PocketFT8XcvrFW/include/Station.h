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
    uint16_t getCursorFreq(void) { return cursorFreq; }     // Hz

    // The setters
    void setCallsign(String s) { callsign = s; }
    void setLocator(String s) { locator = s; }
    void setRig(String s) { rig = s; }
    void setMyName(String s) { myName = s; }
    void setFrequency(unsigned kHz) { frequency = kHz; }
    void setCursorFreq(uint16_t hz) { cursorFreq = hz; }

   private:
    // The station model data
    String callsign;
    String locator;
    String rig;
    String myName;
    unsigned frequency;   // kHz
    uint16_t cursorFreq;  // FSK "tone" offset from carrier in Hz
    //TODO:  move cursor_line here
};