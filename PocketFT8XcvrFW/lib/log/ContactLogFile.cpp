#include <Arduino.h>
#include <string>
#include <unordered_map>

#include "DEBUG.h"
#include "ContactLogFile.h"
#include "lexical.h"


/**
 * @brief Builds the list of known callsigns
 * 
 * Opens and reads the logfile, if it exists, and creates a list of previously contacted callsigns.
 * This list enables (perhaps amongst other things) the Sequencer to ignore dups. 
 */
void ContactLogFile::buildListOfKnownCallsigns() {
    // If it exists, open the log file for reading
    bool err = logFileAdapter.open(this->fileName, MODE_WRITE_FILE);
    if (err) return;  // Doesn't exist or at least isn't readable

    // Loop processing each NL-terminated entry from the previously constructed ADIF file on the SD drive
    char bfr[LOG_ENTRY_SIZE];  // ADIF log entry
    char callsign[16];         // Remote station's callsign
    int count;                 // Number of chars read from ADIF log
    while ((count = logFileAdapter.readLine(bfr, sizeof(bfr))) >= 0) {
        // Spin blank lines
        if (count == 0) continue;

        // Build table of known calls
        if (parseADIF(callsign, bfr, "call", sizeof(callsign)) > 0) {
            DPRINTF("Found callsign '%s' in log\n", callsign);
            knownCallsigns[callsign] = 1;  // The value (1) isn't important
        }
    }
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
    strncap(field, field, sizeof(field));                 // Capitalize field[]
    strncap(capContact, contact, sizeof(capContact));  // Capitalize contact entry

    // Locate field in capitalized contact
    char* pField = strstr(capContact, field);  // Locate first char of token field in contact[] entry
    if (pField == NULL) return 0;              // Not found

    // Parse count of chars in the value
    char* pDigit = pField + strlen(field);        // Locate first digit of count
    unsigned count = 0;                           // Assemble count here
    while ((*pDigit >= '0') && (*pDigit <= 9)) {  // Loop parses digits from count
        count *= 10;                              // Bump previous count before summing new digit
        count += *pDigit - '0';                   // Sum value of newly found digit
        pDigit++;                                 // Bump digit pointer
    }  // At exit, *pCount should point to '>'

    // Validate terminator for count
    if (*pDigit != '>') return 0;  // Malformed ADIF field
    char* pValue = pDigit + 1;     // Locate first char of ADIF value

    // Return token's value to caller
    if (count >= size) return 0;        // Ensure result fits in caller's bfr
    strlcpy(value, pValue, count + 1);  // Copy value chars and NUL to caller's bfr
    value[count] = 0;                   // Let's be damn sure about that NUL
    DPRINTF("count=%d, field='%s', value='%s'\n", count, field, value);
    return count;
} //parseADIF()
