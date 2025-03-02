#pragma once

// Define the Channel message type bits (carried in frame header)
typedef enum {
    CHAN_CRQ = 0b00,   // Connection Request frame
    CHAN_DATA = 0b01,  // Data frame
    CHAN_DISC = 0b10,  // Disconnect request frame
    CHAN_RST = 0b11    // Reset remote endpoint
} ChannelMsgType;

// Define the ARQ message type bits (also in frame header)
typedef enum {
    ARQ_DAT0 = 0b00,  // DATA Sequence 0
    ARQ_ACK0 = 0b01,  // ACK Sequence 0
    ARQ_DAT1 = 0b10,  // DATA Sequence 1
    ARQ_ACK1 = 0b11   // ACK Sequence 1
} ARQMsgType;