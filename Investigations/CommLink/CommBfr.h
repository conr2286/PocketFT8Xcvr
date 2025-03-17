#pragma once

#include "CommLink.h"

class CommBfr {
   public:
    // Define implementation constants
    static const unsigned payloadSize = 32;  // Max number of uncoded bytes in payload (to mostly fit encoded in Serial buffer)

    // Define the layout of a frame header and its payload (Header bits are shared across all layers)
    typedef struct {
        unsigned char phySOM;                // Physical layer prepends its SOM (Start-of-Message) symbol
        unsigned char spare : 1;             // Reserved for future
        unsigned char chnNumber : 2;         // The channel number (WARNING:  See definition of numChannels before changing)
        unsigned char chnMsgHdr : 2;         // The channel's msg header bits
        unsigned char arqMsgHdr : 3;         // The ARQ frame's msg header bits
        uint16_t crc16;                      // 16-bit CRC
        unsigned char payload[payloadSize];  // Uncoded payload bytes
    } FrameType;

    unsigned count;  // Number of bytes in frame

    FrameType frame;  // The uncoded frame (header+payload)

   private:
    CommBfr* next;  // Link to next buffer in FIFO
};

class CommBfrPool {
   public:
    // CommBfrPool(unsigned count);  // Construct a FIFO pool of buffers
    unsigned available();    // Returns number of buffers in the FIFO
    CommBfr* remove();       // Removes the oldest buffer from the FIFO
    void add(CommBfr* bfr);  // Adds newest buffer to the FIFO

   private:
    CommBfr* head;    // Link to first (newest) buffer in FIFO
    CommBfr* tail();  // Find the last (oldest) buffer in FIFO
};