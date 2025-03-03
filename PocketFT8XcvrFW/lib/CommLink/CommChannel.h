#pragma once

#include "CommLink.h"

class CommChannel : Stream {
   private:
    // Define a Channel's connection states
    typedef enum {
        DISCONNECTED = 0,  // Disconnected from remote endpoint
        CONN_PENDING = 1,  // Connection with remote endpoint pending
        CONNECTED = 2,     // Connected with remote endpoint
        DISCO_PENDING = 3  // Disconnect from remote endpoint pending
    } ChannelStateType;

    // Define our implementation constants
    static const unsigned maxChannels = 4;  // Warning:  see dependency in header b4 changing

    // Define our common static variables
    static CommChannel* allChannels[maxChannels];  // One array to in the darkness find them all

    // Define the instance variables unique to each channel
    ChannelStateType state;  // This channel's state

   public:
    // Overide the virtual methods in Stream and Print
    size_t write(uint8_t b) override;  // Write one byte
    int available() override;          // #Bytes that can be read
    int read() override;               // Read one byte from bfr
    int peek() override;               // Peek at (w/o removing) next byte from bfr

    // Expose APIs unique to a CommChannel object
    int availableForWrite();  // #Bytes that can be written
    void connect();           // Establish a connection with remote endpoint
    void disconnect();        // Disconnect from remote endpoint
};
