#include <Arduino.h>

/**
 * Local helper function to pad a char[] to a specified length
 *
 * @param dst  Pointer to the destination char[] string
 * @param src  Pointer to the source char[] string
 * @param c    The pad character (e.g. ' ')
 * @param size sizeof(dst) including the NUL terminator
 *
 * strlpad() copies chars from src[] to dst[], padding dst[] with c to ensure
 * it's filled (including the NUL terminator).  The resulting dst string,
 * including the NUL terminator, will occupy no more than size chars in dst[].
 * 
 * Returns dst or nullptr if error
 *
 **/
char* strlpadch(char* dst, char* src, char c, unsigned size) {
    const char NUL = 0;
    bool paddingUnderway = false;
    unsigned i;

    //Sanity check
    if (size == 0) return nullptr;

    for (i = 0; i < size - 1; i++) {
        if (src[i] == NUL) paddingUnderway = true;
        if (paddingUnderway) {
            dst[i] = c;
        } else {
            dst[i] = src[i];
        }
    }

    dst[size - 1] = NUL;

    return dst;

}  // strlpad()



/**
 * Helper function to pad a char[] to a specified length
 *
 * @param str  Pointer to the char[] string to pad
 * @param size sizeof(str) including the NUL terminator
 * @param c    The pad character (e.g. ' ')
 *
 * strlpad() pads the specified array containing a NUL terminated char string
 * with the specified char, c, ensuring that str remains NUL terminated.  The
 * resulting string, including the NUL terminator, will occupy no more than
 * size chars in str[].
 *
 **/
char* strlpad(char* str, int size, char c) {
    const char NUL = 0;
    bool paddingUnderway = false;
    int i;

    for (i = 0; i < size - 1; i++) {
        if (str[i] == NUL) paddingUnderway = true;
        if (paddingUnderway) {
            str[i] = c;
        }
    }

    str[size - 1] = NUL;

    return str;

}  // strlpad()
