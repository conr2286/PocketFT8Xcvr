#pragma once

#include <Arduino.h>

class CommARQ {
   public:
    // Define the states of the ARQ transmitter
    typedef enum {
        XMIT_RDY0 = 0,   // Ready to xmit DAT0 to remote
        XMIT_WACK0 = 1,  // Waiting to receive ACK0 from remote receiver
        XMIT_RDY1 = 2,   // Ready to xmit DAT1 to remote
        XMIT_WACK1 = 3   // Waiting to receive ACK1 from remote receiver
    } XmitStateType;

    // Define the states of the ARQ receiver
    typedef enum {
        RECV_WDAT0 = 0,  // Waiting to receive DAT0 from remote transmitter
        RECV_RDY0 = 1,   // Waiting to push DAT0 into a channel
        RECV_WDAT1 = 2,  // Waiting to receive DAT1 from remote transmitter
        RECV_RDY1 = 3    // Waiting to push DAT1 into a channel
    } RecvStateType;

    // Define the layout of a frame header and its payload (Header bits are shared across all layers)
    typedef struct {
        unsigned char phySOM;          // Physical layer prepends its SOM (Start-of-Message) symbol
        unsigned char chnNumber : 4;   // The channel number (maxChannels must fit here))
        unsigned char chnMsgType : 2;  // The channel's msg type bits
        unsigned char arqMsgType : 2;  // The ARQ frame's msg type bits
        uint16_t crc16;                // 16-bit CRC
        unsigned char payload[];       // Radix64 Payload bytes (variable length)
        // Note:  Physical layer appends its EOM (End-of-Message) symbol following the payload
    } FrameType;

   private:
    // Define the transmitter and receiver's state variables
    XmitStateType xmitState;  // Transmitter's state
    RecvStateType recvState;  // Receiver's state

    // Define our public member methods
    int sendFrame(FrameType* f);
    int recvFrame(FrameType* f);
};