#pragma once

#include "CommBfr.h"
#include "CommLink.h"

class CommChannel : Stream {
   private:
    // Define the channel message header possibilities in chnMsgHdr
    typedef enum {
        CHN_MSG_HDR_NOP = 0,    // No change in channel state
        CHN_MSG_HDR_CONN = 1,   // Connection request
        CHN_MSG_HDR_DISCO = 2,  // Disconnect request
        CHN_MSG_HDR_RESET = 3   // Reset this channel
    } ChannelMsgType;

    // Define a Channel's connection states
    typedef enum {
        CHN_STATE_DISCO = 0,      // Disconnected from remote endpoint
        CHN_STATE_PEND_CONN = 1,  // Connection with remote endpoint pending
        CHN_STATE_CONN = 2,       // Connected with remote endpoint
        CHN_STATE_PEND_DISCO = 3  // Disconnect from remote endpoint pending
    } ChannelStateType;

    // Define our implementation constants
    static const unsigned numChannels = 2;  // WARNING:  see dependency in FrameType.chnNumber before changing

    // Define the static variables
    static CommChannel* allChannels[numChannels];  // One array to in the darkness find them all

    // Define the instance variables unique to each channel
    ChannelStateType state;     // This channel's state
    CommBfrPool* recvBfrQueue;  // FIFO queue of received buffers  TODO:  new these up!!!
    CommBfr* recvFrame;           // The decoded received frame


   public:
    // Our constructors/destructors
    CommChannel(unsigned channelNumber);
    ~CommChannel();  // Really should implement this someday

    // Overide the virtual methods from Stream and Print
    size_t write(uint8_t b) override;  // Write one byte
    int available() override;          // #Bytes that can be read
    int read() override;               // Read one byte from bfr
    int peek() override;               // Peek at (w/o removing) next byte from bfr

    // Expose APIs unique to a CommChannel object
    int availableForWrite(void);                                 // #Bytes that can be written
    int connect(void);                                           // Establish a connection with remote endpoint
    void disconnect(void);                                       // Disconnect from remote endpoint
    static CommChannel* getCommChannel(unsigned channelNumber);  // Map channel number to a CommChannel pointer
    void addFrame(CommBfr* recvdFrame);                          // Add received frame to channel's inbound FIFO
};
