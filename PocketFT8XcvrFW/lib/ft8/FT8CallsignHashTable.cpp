/**
 * @brief Define the FT8 Hashed Callsign Table implementation
 *
 * BACKGROUND
 *  + FT8 cannot directly convey complex callsigns within its 77-bit fixed message size.
 *    The workaround employs a combination of hashing and compression to support upto
 *    11 character callsigns, still with limitations (noted below).  The interesting use
 *    cases include...
 *
 *  + A non-standard callsign calls CQ (e.g. CQ IAM2ODD) using FT8 message type 4 fields:
 *      h12:    Ignored
 *      c58:    Sender's 11-character (max) non-standard callsign compressed into 58 bits
 *      h1:     Boolean indicates the 2nd (sender's) call is hashed
 *      r2:     Ignored
 *      c1:     Boolean indicates the sender is calling CQ
 *
 *  + A non-standard callsign calls a standard callsign station using FT8 message type 4 fields:
 *      h12:    Sender's callsign hash key
 *      c58:    Sender's 11-character (max) non-standard callsign compressed into 58 bits

 *   + A standard callsign calls a non-standard callsign using FT8 message type 1 (Standard)
 *      c28:    Field1 Recipient's non-standard callsign's hash key
 *      c28:    Field2 Sender's standard 28-bit compressed callsign
 *      r1:     Rover???
 *      R1:     Acknowledgement of a grid locator or signal report
 *
 * LIMITATIONS
 *  + FT8 message type 2 (non-standard) does not support communication between two nonstandard
 *    callsigns.
 *  + Non-standard callsigns are restricted to 11 characters chosen from the Latin aphabet
 *    (e.g. A-Z), the Arabic numerals (0-9), space, and forward slash (/).
 *
 * DESIGN NOTES
 *  FT8 defines the hashing algorithm implemented here in C++ rather than FORTRAN.  The complete
 *  protocol employs 10-bit, 12-bit and 22-bit hashes such that the 10 and 12-bit versions merely
 *  discard the least-significant bits of a 22-bit hash key.  We implement an add() method for
 *  the table using only a 22-bit key and offer retrieval methods for 10, 12 and 22-bit keys.
 *
 *  Surprise:  We don't actually store hashed callsigns in an unordered hash table(!).  The
 *  required 22-bit hash table would have 4,194,307 entries and consume way too
 *  much RAM for our embedded implementation.  We could re-hash the keys with an
 *  unordered c++ map, but we've already invested memory in the ordered map code elsewhere
 *  and aren't keen to invest RAM in both.  We also note that, in practice, an active
 *  communications session likely encounters only a few hundred hashed callsigns so we
 *  don't need the O(1) performance advantage of an unordered (hashed) map.  So... the
 *  ft8NonStandardCallsignTable table is implemented as an ordered map.
 *
 */
#include <Arduino.h>
#include <map>
#include "DEBUG.h"
#include "FT8CallsignHashTable.h"

// Define the implementation of the static error class member
FT8Hash FT8CallsignHashTable::error = 0xffffffff;  // Faked hash key for error indication

// /**
//  * @brief Constructor initializes member variables
//  */
// FT8CallsignHashTable::FT8CallsignHashTable() {
//     FT8CallsignHashTable::error = hashCallsign("error");  // Initialize the error key as a hash of illegal callsign
// }

/**
 * @brief Add an entry to the FT8 nonstandard callsign table
 * @param callsign The callsign text String
 * @return The FT8Hash key value (all available bits)
 *
 * @note FT8 callsign hash keys appear as 10, 12 and 22-bit types, but this implementation
 * always hashes the callsign into the FT8Hash type that can accomodate any FT8 key.  You
 * can convert any native FT8Hash type to a 12-bit key, for example, using shrinkKey12().
 *
 * @note Even though the nonstandard callsign table is implemented as an ordered map rather
 * than an unordered hash table, we still have to hash callsigns because the FT8 protocol
 * transmits the 10, 12 and 22-bit hash keys within its message fields.  Thus, we must
 * use the FT8-defined hash algorithm and we must expose those keys for use by other code.
 */
FT8Hash FT8CallsignHashTable::add(String callsign) {
    DPRINTF("add(%s)\n", callsign.c_str());
    FT8Hash key22 = hashCallsign(callsign);                          // Generate a base-type 22-bit FT8 hash key
    if (key22 != error) nonStandardCallsignTable[key22] = callsign;  // Map the FT8 hash key to the specified callsign
    return key22;                                                    // Return the key or error
}

/**
 * @brief Retrieve a nonstandard callsign string
 * @param key The base key (containing all the implemented bits)
 * @return The nonstandard callsign string (zero-length if the key was not recognized)
 *
 * @note 10-bit and 12-bit keys must be extended to 22-bits
 */
String FT8CallsignHashTable::retrieve(FT8Hash key22) {
    DPRINTF("retrieve(%u)\n", key22);
    return nonStandardCallsignTable[key22];
}

/**
 * @brief Shrink a base key to only 12-bits
 * @param key The base key (containing all 22 implemented bits)
 * @return A 12-bit key
 */
FT8Hash12 FT8CallsignHashTable::shrinkKey12(FT8Hash key) {
    DPRINTF("shrinkKey12(%u)\n", key);
    return key >> 10;  // Really... that's how FT8 makes a 12-bit key from a 22-bit key
}

/**
 * @brief FT8-defined hash function
 * @param s String to be hashed
 * @return A base key (containing all implemented bits) or FT8CallsignHashTable::error
 *
 * @note FT8 expects and we validate the string to be hashed consists of 1..11
 * characters chosen from A-Z, 0-9, ' ' and '/'.
 *
 * @author Derivative of the github ft8_lib save_callsign() implementation
 *
 */
FT8Hash FT8CallsignHashTable::hashCallsign(String s) {
    // Clean the supplied string
    s.trim();                           // Remove leading/trailing spaces
    s.toUpperCase();                    // Force upper-case
    int n = s.length();                 // #Chars in string
    if (n < 1 || n > 11) return error;  // Invalid length

    // Loop extracting each char from the supplied string into a 64-bit hash key
    uint64_t n58 = 0;  // The 64-bit hash key
    int i = 0;         // Indexes each char from string s
    while (i < n) {
        int j = validChars.indexOf(s.charAt(i));  // Lookup the index of i'th char in validChars
        if (j < 0) return error;                  // hash error (wrong character set)
        n58 = (38 * n58) + j;                     // Process charAt(i) into hash
        i++;
    }

    // pretend to have trailing whitespace (with j=0, index of ' ')
    while (i < 11) {
        n58 = (38 * n58);
        i++;
    }

    // Calculate the 32-bit base hash key a la other FT8 implementations
    FT8Hash n22 = ((47055833459ull * n58) >> (64 - 22)) & (0x3FFFFFul);
    DPRINTF("Hashed '%s' to %lu\n", s.c_str(), n22);

    return n22;  // We always and only return the 22-bit base result
}
