#pragma once

#include <stddef.h>
#include <stdint.h>

// For a native test build, we need to mock-up the Stream class to enable CommStream to compile.
// Here we are cheating a bit, defining virtuals for both Stream and its base class, Print.
class Stream {
   public:
    // Define the virtual methods of the Arduino Stream (and Print)
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t write(uint8_t b) = 0;
};