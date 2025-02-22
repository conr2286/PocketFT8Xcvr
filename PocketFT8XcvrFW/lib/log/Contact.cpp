/**
 * Contact --- Collects details about a single contact (QSO)
 *
 * Usage:  Normally, a station has but a single active contact at any
 * moment in time.  However, this implementation does not prevent you
 * from instantiating more than one contact if that's somehow useful.
 *
 * The begin() method initializes a Contact for a new QSO after which
 * the fields may be supplied by setters.
 *
 * To log a Contact, pass it as a parameter to a LogFile object.
 *
 * A Contact's reset() method resets a Contact's fields to empty strings.
 *
 *
 * The required members for a valid (completed) contact are:
 *  + workedCall    Worked (remote) station's callsign
 *  + myCall        Our station operator's callsign
 *  + qsoDate       The UTC date (YYYYMMDD) when QSO began
 *  + qsoTime       The UTC time (HHMMSS) when QSO began
 *  + freq          The QSO frequency (kHz)
 *  + mode          The modulation mode (e.g. FT8)
 *  + workedRSL     Worked station's signal report send
 *. + myRSL         Our station's signal report received
 *
 * Optional members include:
 *  + workedLocator Worked station's (4 or 6 letter) grid square
 *  + myLocator     Our station's (4 or 6 letter) grid square
 *  + workedRSL     Worked station's Received Signal Level
 *  + myRSL         Our station's Received Signal Level report
 *
 * A contact has a defined beginning after which data may be supplied for fields
 * as they become available during the QSO.  Signal reports are required for a
 * valid contact to ensure that the qso was actually completed successfully;
 * note that this is a bit more restrictive than LoTW which does not require
 * an exchange of signal reports.
 *
 * Limitations:  The mapping of frequency to band is currently implemented
 * only for 160m, 80m, 40m, 30m, 20m, 17m, 15m, 12m, and 10m.
 *
 * Design Notes:  Contact deals with the acquisition of data about a
 * single QSO, but is not responsible for recording that QSO in a file.
 * Recording Contacts in a collection is a service provided by LogFile.
 * All Contact fields are recorded as text strings in char[] arrays.
 * Contact fields are reasonably independent of any particular log file
 * format.  However, when an encoding style must be chosen, we've
 * chosen ADIF.  For example, the date string is recorded as UTC YYYYMMDD
 * rather than any of the myriad ways we might encode today's date.
 *
 * Kludge:  The oddEven field is unique to FT8 and similar digital modes
 * using robo-sequencing through QSO messages.  It likely has nothing to
 * do with logging a QSO.  We justify including it here because it is
 * indeed instance data for a QSO.
 *
 * Someday:  Consider implementing Contact as key/value pairs to provide
 * extensibility of the field (key) names.
 *
 **/
#include "Contact.h"

#include <TimeLib.h>
#include <stdio.h>
#include <string.h>

#include "DEBUG.h"

// Externs
extern char Station_Call[];

/**
 *  Begin recording a contact
 *
 * @param workedCall  Remote station's callsign
 * @param myCall      Our station operator's callsign
 * @param freq        Frequency in kHz
 * @param mode        Modulation mode (e.g. FT8 or DATA)
 * @param oddEven     Remote station transmits in 1==odd, 0==even-numbered timeslots
 *
 * Notes:  begin() converts freq to the ADIF-compliant band enumeration.  begin() records
 * the date and time in the ADIF-compliant encoding.  begin() resets all unsupplied
 * fields to empty strings.
 *
 * Overwrites existing field data, if any, in an active Contact instance afterwhich
 * active will become true.
 *
 * WARNING:  Not all amateur frequencies are converted to ADIF-compliant band enumerations
 * at this version of the code.
 *
 **/
void Contact::begin(char* workedCall, unsigned freq, const char* mode, unsigned oddEven) {
    char* myCall = Station_Call;  // TODO:  Seek alternatives to these externs

    // DPRINTF("workedCall=%s, myCall=%s, freq=%u, mode=%s, oddEven=%u\n", workedCall, myCall, freq, mode, oddEven);

    // Reset all Contact field values (except odd/even which has no "undefined" value)
    reset();

    // Record the supplied char fields
    strlcpy(this->workedCall, workedCall, sizeof(this->workedCall));
    strlcpy(this->myCall, myCall, sizeof(this->myCall));
    strlcpy(this->mode, mode, sizeof(this->mode));

    // Record whether remote station transmits in an odd or even numbered sequence
    this->oddEven = oddEven & 0x01;  // Force binary value:  1==odd, 0==even

    // Get the current Teensy date and time
    time_t t = now();

    // Supply the date entry in the ADIF-compliant format (e.g. YYYYMMDD)
    snprintf(this->qsoDate, sizeof(this->qsoDate), "%04d%02d%02d", year(t), month(t), day(t));

    // Supply the time entry in the ADIF-compliant format (e.g. HHMMSS)
    snprintf(this->qsoTime, sizeof(this->qsoTime), "%02d%02d%02d", hour(t), minute(t), second(t));

    // Convert the supplied frequency into the ADIF-compliant band format (e.g. 40m)
    if (freq >= 1800 && freq <= 2000) {
        strlcpy(this->band, "160m", sizeof(this->band));
    } else if (freq >= 3500 && freq <= 4000) {
        strlcpy(this->band, "80m", sizeof(this->band));
    } else if (freq >= 7000 && freq <= 7300) {
        strlcpy(this->band, "40m", sizeof(this->band));
    } else if (freq >= 10100 && freq <= 10150) {
        strlcpy(this->band, "30m", sizeof(this->band));
    } else if (freq >= 14000 && freq <= 14350) {
        strlcpy(this->band, "20m", sizeof(this->band));
    } else if (freq >= 18068 && freq <= 18168) {
        strlcpy(this->band, "17m", sizeof(this->band));
    } else if (freq >= 21000 && freq <= 21450) {
        strlcpy(this->band, "15m", sizeof(this->band));
    } else if (freq >= 24890 && freq <= 24990) {
        strlcpy(this->band, "12m", sizeof(this->band));
    } else if (freq >= 28000 && freq <= 29700) {
        strlcpy(this->band, "10m", sizeof(this->band));
    }

    active = true;  // Contact has begun actively recording QSO data

}  // begin()

/**
 * Reset Contact log data fields to empty strings
 *
 * Marks a Contact as inactive
 *
 **/
void Contact::reset() {
    this->workedCall[0] = 0;
    this->myCall[0] = 0;
    this->qsoDate[0] = 0;
    this->qsoTime[0] = 0;
    this->band[0] = 0;
    this->mode[0] = 0;
    this->workedRSL[0] = 0;
    this->myRSL[0] = 0;
    this->mySOTAref[0] = 0;
    this->workedSOTAref[0] = 0;
    this->myLocator[0] = 0;
    this->workedLocator[0] = 0;
    this->myRig[0] = 0;
    this->txPwr[0] = 0;
    this->active = false;
}  // reset()

/**
 * Determine if a Contact is actively recording data about a QSO
 *
 *
 **/
bool Contact::isActive() {
    // DPRINTF("isActive()=%u\n", this->active);
    return this->active;
}

/**
 *  Determine if a contact is valid (completed)
 *
 * @return true if the Contact contains all required fields, false otherwise
 *
 * Design notes:  Currently does not inspect the active field
 *
 **/
bool Contact::isValid() {
    // DFPRINTF("workedCall='%s', myCall='%s', band='%s', mode='%s', qsoDate='%s', qsoTime='%s', workedRSL='%s', myRSL='%s')\n", this->workedCall, this->myCall, this->band, this->mode,this->qsoDate,this->qsoTime,this->workedRSL,this->myRSL);
    bool result = strlen(this->workedCall) > 0 && strlen(this->myCall) > 0 && strlen(this->band) > 0 && strlen(this->mode) > 0 && strlen(this->qsoDate) > 0 && strlen(this->qsoTime) > 0 && strlen(this->workedRSL) > 0 && strlen(this->myRSL) > 0;
    // DPRINTF("isValid()=%u\n", result);
    return result;
}  // isValid()

// Here are the setters for the fields that get filled-in after begin()
void Contact::setWorkedRSL(char* rsl) {
    strlcpy(this->workedRSL, rsl, sizeof(this->workedRSL));
}

void Contact::setWorkedRSL(int rsl) {
    snprintf(this->workedRSL, sizeof(this->workedRSL), "%d", rsl);
}

void Contact::setMyRSL(char* rsl) {
    strlcpy(this->myRSL, rsl, sizeof(this->myRSL));
}

void Contact::setMyLocator(char* locator) {
    strlcpy(this->myLocator, locator, sizeof(this->myLocator));
}

void Contact::setWorkedLocator(char* locator) {
    strlcpy(this->workedLocator, locator, sizeof(this->workedLocator));
}

void Contact::setMySOTAref(char* sotaRef) {
    strlcpy(this->mySOTAref, sotaRef, sizeof(this->mySOTAref));
}

void Contact::setWorkedSOTAref(char* sotaRef) {
    strlcpy(this->workedSOTAref, sotaRef, sizeof(this->workedSOTAref));
}

void Contact::setRig(const char* rig) {
    strlcpy(this->myRig, rig, sizeof(this->myRig));
}

void Contact::setPwr(float pwr) {
    snprintf(this->txPwr, sizeof(this->txPwr), "%f", pwr);
}

// Here are the getters for the fields
char* Contact::getRig() {
    return this->myRig;
}

char* Contact::getPwr() {
    return this->txPwr;
}

char* Contact::getWorkedCall() {
    return this->workedCall;
}

char* Contact::getMyCall() {
    return this->myCall;
}

char* Contact::getQSOdate() {
    return this->qsoDate;
}

char* Contact::getQSOtime() {
    return this->qsoTime;
}

char* Contact::getBand() {
    return this->band;
}

char* Contact::getMode() {
    return this->mode;
}

char* Contact::getMyRSL() {
    return this->myRSL;
}

char* Contact::getWorkedRSL() {
    return this->workedRSL;
}

char* Contact::getMyLocator() {
    return this->myLocator;
}

char* Contact::getWorkedLocator() {
    return this->workedLocator;
}

char* Contact::getMySOTAref() {
    return this->mySOTAref;
}

char* Contact::getWorkedSOTAref() {
    return this->workedSOTAref;
}
