#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

class Radix64 {
   public:
    static char *encodeRadix64(char* dst, const char *src, size_t input_length);
    static char *decodeRadix64(char* dst, const char *src, size_t *output_length);

    private:
};
