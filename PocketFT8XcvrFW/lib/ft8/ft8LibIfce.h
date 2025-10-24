#pragma once

#include <stdint.h>
#include "msgTypes.h"

/**
 * @brief Define the legacy ft8_lib interface
 *
 * @note The ft8_lib underwent substantial refactoring and enhancements over the years.  This
 * interface implements a legacy view of the revised (2025) ft8_lib code.
 *
 * field1 char[14] Commonly used for a destination station's callsign
 * field2 char[14] Commonly used for the source station's callsign
 * field3 char[7]  Commonly used for the extra info (e.g. 73, R73, RR73, locator...)
 * a77 char[10] FT8 packed message bits
 *
 */

// Unpacks a 77 bit demodulated message into three FT8 fields
int unpack77_fields(const uint8_t* a77, char* field1, char* field2, char* field3, MsgType* msgType);

// Unpacks a 77-bit message (e.g. free text) into a single char[35] string
int unpack77(const uint8_t* a77, char* message);

// Pack any supported FT8 text message into a 77-bit array
int pack77(const char* msg, uint8_t* a77);  // Pack any supported FT8 message into c77

// Unlike trimCallsign(), this function trims brackets in-place from a callsign string
void trimBracketsFromCallsign(char* s);  // Trims angle brackets from callsign in-place
