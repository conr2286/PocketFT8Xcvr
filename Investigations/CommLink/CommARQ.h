#pragma once

#include <Arduino.h>

#include "CommBfr.h"
#include "CommChannel.h"
#include "CommPhy.h"

class CommARQ {
   public:
    // Implementation constants
    static const char EOM = '\n';                                 // End-of-Message (frame) terminator
    static const unsigned phyBfrSize = CommBfr::payloadSize * 2;  // Number bytes in physical link's buffer

    // Define the states of the ARQ transmitter
    typedef enum {
        XMIT_STATE_RDY0 = 0,   // Ready to xmit DAT0 to remote
        XMIT_STATE_WACK0 = 1,  // Waiting to receive ACK0 from remote receiver
        XMIT_STATE_RDY1 = 2,   // Ready to xmit DAT1 to remote
        XMIT_STATE_WACK1 = 3   // Waiting to receive ACK1 from remote receiver
    } XmitStateType;

    // Define the states of the ARQ receiver
    typedef enum {
        RECV_STATE_WDAT0 = 0,  // Waiting to receive DAT0 from remote transmitter
        RECV_STATE_BFR0 = 1,   // Waiting for channel to return BFR so we can xmit ACK0
        RECV_STATE_WDAT1 = 2,  // Waiting to receive DAT1 from remote transmitter
        RECV_STATE_BFR1 = 3    // Waiting for channel to return BFR so we can xmit ACK1
    } RecvStateType;

    // Define our public methods
    CommARQ(CommPhy* phyLink);   // Constructor binds ARQ to a physical device
    void poll(void);             // Service the ARQ layer (deals with received frames)
    void freeBfr(CommBfr* bfr);  // Add specified buffer to receiver's freeBfrPool
    void sendBfr(CommBfr* bfr);  // Transmit specified buffer to remote

   private:
    // Define the transmitter and receiver's state variables
    XmitStateType xmitState;  // Transmitter's state
    RecvStateType recvState;  // Receiver's state

    // Other private member variables
    CommPhy* commPhy;  // Binding to the physical link transmitter/receiver

    // Receiver's private members
    char phyRecvBfr[phyBfrSize];  // Receiver collects phys link's bytes here
    CommBfrPool recvFreePool;     // Receiver's pool of free frame buffers

    // Transmitter's private members
    CommBfrPool xmitQueue;        // Transmitter's outbound queue
    char phyXmitBfr[phyBfrSize];  // Transmitters phys link buffer

    // Private helper methods
    void processReceivedChar(char c);
    CommBfr* decodeFrame();                         // Decode Radix64 data from phyBfr
    void sendChannelReset(unsigned channelNumber);  // Send RST frame
    void resetCommLink(void);                       // Reset everything
    bool verifyCRC(CommBfr* recvFrame);             // Verify calc'd CRC with recv'd CRC

    void recvDAT0(CommChannel* chn, CommBfr* bfr);  // Process received DAT0 frame
    void recvDAT1(CommChannel* chn, CommBfr* bfr);  // Process received DAT1 frame
    void recvACK0(void);                            // Process received ACK0
    void recvACK1(void);                            // Process received ACK1

    void xmitDAT0(void);  // Transmit DAT0 frame
    void xmitDAT1(void);  // Transmit DAT1 frame
    void xmitACK0(void);  // Transmit an ACK0
    void xmitACK1(void);  // Transmit an ACK1
};