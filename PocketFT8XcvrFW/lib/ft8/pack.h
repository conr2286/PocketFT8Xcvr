#pragma once

#include <stdint.h>

// Pack FT8 text message into 72 bits
// [IN] msg      - FT8 message (e.g. "CQ TE5T KN01")
// [OUT] c77     - 10 byte array to store the 77 bit payload (MSB first)
int pack77(const char* msg, uint8_t* c77);        // Pack any supported FT8 message into c77
void packtext77(const char* text, uint8_t* c77);  // Pack FT8 free text message into c77
int pack77_1(const char* msg, uint8_t* c77);      // Pack FT8 standard message intot c77
