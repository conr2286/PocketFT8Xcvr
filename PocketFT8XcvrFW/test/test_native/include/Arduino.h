/**
 * This is a mock Arduino.h file enabling some unit tests to compile and execute on native environment
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

class MockSerial {
   public:
    int printf(const char* format, ...);
};

extern MockSerial Serial;
