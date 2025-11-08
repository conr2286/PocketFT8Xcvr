#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <string>
#include "Arduino.h"

int foo;

MockSerial Serial;

int MockSerial::printf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int retval = vprintf(format, ap);
    va_end(ap);
    return retval;
}
