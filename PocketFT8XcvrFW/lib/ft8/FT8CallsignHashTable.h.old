#pragma once
/**
 * FT8 callsign hash table declarations
 */
#include <Arduino.h>
#include <map>

typedef uint32_t FT8Hash;   // The base type used for all FT8 hash keys
typedef FT8Hash FT8Hash10;  // The 10-bit hash key
typedef FT8Hash FT8Hash12;  // The 12-bit hash key
typedef FT8Hash FT8Hash22;  // The 22-bit hash key

// Declare the FT8 Hashed Callsign Map Table (it really should be a singleton)
class FT8CallsignHashTable {
   public:
    FT8Hash12 add12(String callsign);  // Add a callsign to the table returning a 12-bit key
    FT8Hash12 add22(String callsign);  // Add a callsign to the table returning a 22-bit key
    String lookup(FT8Hash key);        // Retrieve a callsign from table
    static FT8Hash error;              // Special hash key indicating an error

   private:
    std::map<FT8Hash, String> nonStandardCallsignTable;            // Surprise:  Implemented as an ordered map!
    FT8Hash hashCallsign(String s);                                // The FT8-defined hash function
    String validChars = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";  // Valid nonstandard callsign chars
};