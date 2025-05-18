/**
 * @brief Station model
 *
 * Defines many operating parameters for the current station.  Unlike the content
 * of the config structure (which is simply a RAM-resident copy of CONFIG.JSON),
 * the content of Station potentially can vary during execution.
 *
 */
#pragma once
#include <Arduino.h>

class Station {
   public:
    Station(unsigned minF, unsigned maxF) { 
        minFreq = minF;
        maxFreq = maxF;
    }

    // Determine if this Station has everything required to transmit
    bool canTransmit(void) {
        if (((frequency >= minFreq) && (frequency <= maxFreq)) && (strlen(callsign.c_str()) > 0) && (strlen(locator.c_str()) > 0))
            return true;
        else
            return false;
    }  // canTransmit()

    // The getters
    const char* getCallsign(void) { return callsign.c_str(); }
    const char* getLocator(void) { return locator.c_str(); }
    const char* getRig(void) { return rig.c_str(); }
    const char* getMyName(void) { return myName.c_str(); }  // Operator's name, not callsign
    unsigned getFrequency(void) { return frequency; }       // kHz
    uint16_t getCursorFreq(void) { return cursorFreq; }     // Hz
    bool getEnableDuplicates(void) { return enableDuplicates; }
    bool getEnableTransmit(void) { return enableTransmit; }
    unsigned getQSOtimeout(void) { return qsoTimeout; }

    // The setters
    void setCallsign(String s) { callsign = s; }
    void setLocator(String s) { locator = s; }
    void setRig(String s) { rig = s; }
    void setMyName(String s) { myName = s; }
    void setFrequency(unsigned kHz) { frequency = kHz; }
    void setCursorFreq(uint16_t hz) { cursorFreq = hz; }
    void setEnableDuplicates(bool enabled) { enableDuplicates = enabled; }
    void setEnableTransmit(bool enabled) { enableTransmit = enabled; }
    void setQSOtimeout(unsigned seconds) { qsoTimeout = seconds; }

   private:
    String callsign;        // My callsign
    String locator;         // My Maidenhead Gridsquare
    String rig;             // My rig
    String myName;          // My name
    unsigned minFreq;       // Minimum freq supported by HW filters
    unsigned maxFreq;       // Maximum freq supported by HW filters
    unsigned frequency;     // Carrier kHz
    uint16_t cursorFreq;    // FSK "tone" offset from carrier in Hz
    bool enableDuplicates;  // true==>enable RoboOp to respond to duplicate (previously logged) callsigns
    bool enableTransmit;    // true==>enable transmitter
    unsigned qsoTimeout;    // QSO timeout seconds
    // TODO:  move cursor_line here
};