#include <Arduino.h>
#include <map>
#include "CallsignHashTable.h"

/**
 * @brief Record an entry in the nonStandardCallsignTable map for key
 * @param callsign The callsign to be associated with key
 * @param key The key
 *
 * @note We don't actually use a hash table as it would consume quite a bit of memory
 * unless we rehashed, and Pocket FT8 already used the ordered map elsewhere
 *
 * @note Recording an entry with a previously used key will overwrite the existing entry
 */
void CallsignHashTable::add(char* callsign, uint32_t key) {
    nonStandardCallsignTable[key] = callsign;
}  // add()

/**
 * @brief Lookup an entry in the nonStandardCallsignTable for key
 * @param hash_type Ignored
 * @param key The key
 * @param c11 Buffer to receive the callsign
 * @return true==success
 */
bool CallsignHashTable::lookup(ftx_callsign_hash_type_t hash_type, uint32_t key, char* c11) {
    String s = nonStandardCallsignTable[key];
    if (s.length() > 0) {
        strlcpy(c11, s.c_str(), 12);
        return true;
    } else {
        return false;
    }
}  // lookup()