#include "ContactLogFile.h"

#include <Arduino.h>

#include <map>
#include <string>

#include "DEBUG.h"
#include "lexical.h"

//This is the hash table for known (previously logged) callsigns.  We new-up the table so
//the required memory will be allocated from Teensy RAM2 rather than RAM1.  Note:  unlike
//the FT8 callsign hash table (concerned with what callsigns have been *heard*), this table
//records whether our station has actually attempted to *work* the remote station.
static bool* knownCallsigns;  // The callsign hash table

/**
 * @brief Add callsign to list of previously logged callsigns
 * @param callsign NUL-terminated char[] of callsign
 */
void ContactLogFile::addKnownCallsign(char* callsign) {
    unsigned callsignHash = hashString(callsign);
    DPRINTF("Add known callsign: '%s', callsignHash=%u\n", callsign, callsignHash);
    knownCallsigns[callsignHash] = true;
}

/**
 * @brief Builds the list of known callsigns
 *
 * Opens and reads the logfile, if it exists, and creates a list of previously contacted callsigns.
 * This list enables (perhaps amongst other things) the Sequencer to ignore dups.
 */
void ContactLogFile::buildListOfKnownCallsigns() {
    DTRACE();

    // The known callsign table has 0..callsignTableSize-1 boolean entries, indexed by
    // a hash key.  true implies we've seen this hash before, false implies we have not.
    knownCallsigns = new bool[callsignTableSize];
    for (unsigned i = 0; i < callsignTableSize; i++) knownCallsigns[i] = false;

    // If it exists, open the log file for reading
    bool err = logFileAdapter.open(this->fileName, MODE_READ_FILE);
    if (err) return;  // Doesn't exist or at least isn't readable

    // DTRACE();

    // Loop processing each NL-terminated entry from the previously constructed ADIF file on the SD drive
    char bfr[LOG_ENTRY_SIZE];  // ADIF log entry
    char callsign[16];         // Remote station's callsign
    int count;                 // Number of chars read from ADIF log
    while ((count = logFileAdapter.readLine(bfr, sizeof(bfr))) >= 0) {
        // DPRINTF("count=%d, bfr='%s'\n", count,bfr);
        //  Spin blank lines
        if (count == 0) continue;

        // Build table of known calls
        if (parseADIF(callsign, bfr, "call", sizeof(callsign)) > 0) {
            addKnownCallsign(callsign);
        }
        // DTRACE();
    }

    logFileAdapter.close();
    // DTRACE();
}  // buildListOfKnownCallsigns()

/**
 * @brief Compute a hash key for the specified char[] string
 * @param str NUL-terminated char[] string
 * @return The hash key
 *
 * The resulting hash key will lie in the range 0..callsignTableSize-1
 *
 * Method:  Modulo division by a prime number
 */
unsigned ContactLogFile::hashString(char* str) {
    unsigned result = 0;
    for (char* p = str; *p != 0; p++) {
        result += *p;
    }
    return (result % callsignTableSize);  // This is the hash function
}  // hashString()

/**
 * @brief Determine if specified callsign has been previously logged
 * @param callsign NUL-terminated char[] string containing a callsign
 * @return true if previously seen, else false
 *
 * Method:  Very simple hashing --- a key collision will return a false
 * positive for a station which actually isn't in the log.  Note that the
 * station can still be worked manually (click on their CQ msg).  TODO:  we
 * really should handle collisions better.
 */
bool ContactLogFile::isKnownCallsign(char* callsign) {
    unsigned hashkey = hashString(callsign);
    bool result = knownCallsigns[hashkey];
    DPRINTF("hashkey=%u, isKnownCallsign('%s')=%u\n", hashkey,callsign, result);
    return result;
}

/**
 * @brief Parse value of token from an ADIF contact entry
 * @param value Destination bfr to receive token's value
 * @param contact ADIF entry for a single contact
 * @param token The desired ADIF field name
 * @param size  sizeof(value) array
 * @return Number of chars, excluding NUL terminator, placed in value[]
 *
 * NOTE:  An ADIF entry is a NUL-terminated char[] string forming a key value map of
 * ADIF fields to their values.  Example key value entry for the remote station's call:
 *      <call:5>AE0TB
 * In the above, <call:5> identifies this token as the ADIF key for the remote station
 * and specifies that its value consists of 5 chars.  AE0TB is the 5-char value.
 *
 * USAGE:  parseADIF(remoteStationCall, adifContact, "call") where:
 *
 *          remoteStationCall:  char* to buffer to receive remote station's callsign
 *          adifContact:  char* to the complete ADIF contact entry
 *          "call":  Identifies the desired ADIF field
 *
 * Normally returns a NUL terminated char[] string.  If anything goes wrong, the
 * result will be an empty string (just a NUL).
 *
 */
int ContactLogFile::parseADIF(char* value, char* contact, const char* token, unsigned size) {
    // Rule-out craziness
    if ((value == NULL) || (size == 0)) return 0;  // Hopeless
    value[0] = 0;                                  // Terminate result with NUL in case we bale out below

    // Validate remaining parameters
    if ((contact == NULL) || (token == NULL)) return 0;

    // Build ADIF field in contact
    char field[FIELD_SIZE];             // We will build field here
    strlcpy(field, "<", FIELD_SIZE);    // ADIF entries always begin with angle bracket
    strlcat(field, token, FIELD_SIZE);  // Name of desired field
    strlcat(field, ":", FIELD_SIZE);    // Name always ends with a colon

    // Convert everything to upper-case prior to search
    char capContact[LOG_ENTRY_SIZE];                   // Capitalized contact[] entry
    strncap(field, field, sizeof(field));              // Capitalize field[]
    strncap(capContact, contact, sizeof(capContact));  // Capitalize contact entry

    // Locate field in capitalized contact
    // DPRINTF("parseADIF field='%s' capContact='%s'\n", field, capContact);
    char* pField = strstr(capContact, field);  // Locate first char of token field in contact[] entry
    if (pField == NULL) return 0;              // Not found
    // DTRACE();

    // Parse count of chars in the value
    char* pDigit = pField + strlen(field);  // Locate first digit of count
    unsigned count = 0;                     // Assemble count here
    // DPRINTF("*pDigit='%c'\n", *pDigit);
    while ((*pDigit >= '0') && (*pDigit <= '9')) {  // Loop parses digits from count
        count *= 10;                                // Bump previous count before summing new digit
        count += *pDigit - '0';                     // Sum value of newly found digit
        pDigit++;                                   // Bump digit pointer
    }  // At exit, *pDigit should point to '>'
    // DPRINTF("count=%d, *pDigit='%c'", count, *pDigit);

    // Validate terminator for count
    if (*pDigit != '>') return 0;  // Malformed ADIF field
    char* pValue = pDigit + 1;     // Locate first char of ADIF value
    // DPRINTF("*pValue='%c'\n", *pValue);

    // Return token's value to caller
    if (count >= size) return 0;        // Ensure result fits in caller's bfr
    strlcpy(value, pValue, count + 1);  // Copy value chars and NUL to caller's bfr
    value[count] = 0;                   // Let's be damn sure about that NUL
    // DPRINTF("count=%d, field='%s', value='%s'\n", count, field, value);
    return count;
}  // parseADIF()
