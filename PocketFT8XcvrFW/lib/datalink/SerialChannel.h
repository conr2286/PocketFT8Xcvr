#pragma once

#include <Arduino.h>

class SerialChannel : public Stream {
   public:
    int available();
    int read();
    int peek();
    size_t write(char c);
    size_t write(char* str);
    size_t write(char* bfr, size_t len);

   private:

   //Transmitter endpoint's states
    typedef enum { XMIT_S0,     //Idle
                   XMIT_S1,
                   XMIT_S2,
                   XMIT_S3 } XmitStateType;

    //Receiver endpoint's states
    typedef enum { RECV_S0,     //Idle
                   RECV_S1,
                   RECV_S2,
                   RECV_S3 } RecvStateType;

    // Implementation constants
    static const unsigned nChannels = 4;            //Maximum number of SerialChannel instances 

    // Static vars
    static SerialChannel* allChannels[nChannels];   //Pointer to each SerialChannel instance
    static unsigned bfrSize;                        //sizeof xmitBfr and recvBfr determined at runtime

    //State variables for each SerialChannel's transmitter and receiver endpoint
    XmitStateType xmitState;    //This channel's transmitter state
    RecvStateType recvState;    //This channel's receiver state

    //Each SerialChannel has a transmitter and receiver buffer allocated at runtime
    char* xmitBfr;      //Transmitter's buffer
    char* recvBfr;      //Receiver's buffer
};
