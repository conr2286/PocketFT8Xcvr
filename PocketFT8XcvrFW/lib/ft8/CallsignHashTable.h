#pragma once
/**
 * FT8 callsign hash table declarations
 */
#include <Arduino.h>
#include <map>
#include "message.h"

// Declare the FT8 Hashed Callsign Map Table (it really should be a singleton)
class CallsignHashTable {
   public:
    void add(char* callsign, uint32_t key);  // Add a callsign to the table using specified FT8 key
    bool lookup(ftx_callsign_hash_type_t hash_type, uint32_t hash, char* c11);

   private:
    std::map<uint32_t, String> nonStandardCallsignTable;  // Surprise:  Implemented as an ordered map!
};