#include <string.h>
#include <ctype.h>

#include "lexical.h"

/**
 * @brief Capitalize a char[] string
 * @param dst Destination char[] string
 * @param src Source char[] string
 * @param size sizeof(dst) including NUL terminator
 *
 * strncap capitalizes the src string, placing the result in dst, respecting
 * the sizeof(dst).  Both src and dst may point to the same area in memory
 * to capitalize a string in place.  All char[] strings are NUL terminated.
 */
void strncap(char* dst, char* src, unsigned size) {
    // Sanity checks
    if ((dst == NULL) || (src == NULL) || (size == 0)) return;

    // Capitalize each char from src, placing the result in dst, attending to size
    unsigned count = 0;
    while ((*src != 0) && (count < size)) {
        *dst++ = toupper(*src++);  // Convert this char to uppercase
        count++;                   // and mind the count
    }
    *dst = 0;  // NUL terminator
}





