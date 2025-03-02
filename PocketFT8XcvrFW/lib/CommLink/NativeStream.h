#pragma once

#include <stdint.h>

class Stream {
   public:
    // Define the virtual methods of the Arduino Stream (and Print)
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t write(uint8_t b) = 0;
};