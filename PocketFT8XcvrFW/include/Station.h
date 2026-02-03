/**
 * @brief Station models many attributes of our amateur radio station.
 *
 * @note Unlike the content of the config structure (which is simply a
 * RAM-resident copy of CONFIG.JSON), the content of Station potentially
 * can vary during execution.
 *
 * @note Station is implemented as a Meyers Singleton.  You can obtain a
 * reference to the one-and-only instance using Station:getInstance().
 *
 */
#pragma once
#include <Arduino.h>

class Station {
   public:
    // Implementation of the singleton's getter
    static Station& getInstance() {
        static Station theInstance;  // This is the one-and-only instance of the Station class
        return theInstance;          // Return a reference to the one-and-only Station object
    }  // getInstance()

    // Determine if this Station has everything required to transmit
    bool canTransmit(void) {
        // DPRINTF("operatingFrequency=%u, callsign=%s, locator=%s\n", operatingFrequency, callsign.c_str(), locator.c_str());
        if ((operatingFrequency > 0) && (strlen(callsign.c_str()) > 0) && (strlen(locator.c_str()) > 0)) {
            return true;
        } else {
            return false;
        }
    }  // canTransmit()

    // The getters
    const char* getCallsign(void) { return callsign.c_str(); }
    const char* getLocator(void) { return locator.c_str(); }
    const char* getSOTAref(void) { return mySOTAref.c_str(); }
    const char* getRig(void) { return rig.c_str(); }
    const char* getMyName(void) { return myName.c_str(); }      // Operator's name, not callsign
    unsigned getFrequency(void) { return operatingFrequency; }  // kHz
    uint16_t getCursorFreq(void) { return cursorFreq; }         // Hz
    bool getEnableDuplicates(void) { return enableDuplicates; }
    bool getEnableTransmit(void) { return enableTransmit; }
    unsigned getQSOtimeout(void) { return qsoTimeout; }

    // The setters
    void setCallsign(String s) { callsign = s; }
    void setLocator(String s) { locator = s; }
    void setRig(String s) { rig = s; }
    void setSOTAref(String s) { mySOTAref = s; }
    void setMyName(String s) { myName = s; }
    void setFrequency(unsigned kHz) { operatingFrequency = kHz; }
    void setCursorFreq(uint16_t hz) { cursorFreq = hz; }
    void setEnableDuplicates(bool enabled) { enableDuplicates = enabled; }
    void setEnableTransmit(bool enabled) { enableTransmit = enabled; }
    void setQSOtimeout(unsigned seconds) { qsoTimeout = seconds; }

   private:
    // Methods etc associated with the Meyers singleton implementation
    Station() : operatingFrequency(0), cursorFreq(0), qsoTimeout(0), enableDuplicates(false), enableTransmit(false) {
    }  // Station()
    Station(const Station&) = delete;             // Delete singleton's copy constructor
    Station& operator=(const Station&) = delete;  // Delete assignment operator

    // Station attributes
    String callsign;   // My callsign
    String locator;    // My Maidenhead Gridsquare
    String rig;        // My rig
    String myName;     // My name
    String mySOTAref;  // This station's SOTA Reference number
    // unsigned minFreq;             // Minimum freq supported by HW filters
    // unsigned maxFreq;             // Maximum freq supported by HW filters
    unsigned operatingFrequency;  // Carrier kHz
    uint16_t cursorFreq;          // FSK "tone" offset from carrier in Hz
    unsigned qsoTimeout;          // QSO timeout seconds
    bool enableDuplicates;        // true==>enable RoboOp to respond to duplicate (previously logged) callsigns
    bool enableTransmit;          // true==>enable transmitter
};