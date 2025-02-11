#pragma once

#include "Contact.h"
#include "SequencerStates.h"
#include "Timer.h"
#include "decode_ft8.h"

class Sequencer {
   private:
    // Information saved about a QSO
    Contact contact;     // Info gathered about the QSO
    char theirTimeslot;  // 0==even, 1==odd

    // The Sequencer singleton's private constructor
    Sequencer() {
        sequenceNumber = 0;
    }

    // Delete copy constructor and assignment operator to prevent copying
    // Sequencer(const Sequencer &) = delete;
    // Sequencer &operator=(const Sequencer &) = delete;

    // Define the events arising from analysis of received messages
    void locatorEvent(Decode *msg);        // Received their maidenhead locator in msg
    void rslEvent(Decode *msg);            // Received our signal report in msg
    void eotEvent(Decode *msg);            // Received an EOT (e.g. 73) that doesn't expect a reply
    void eotReplyEvent(Decode *msg);       // Received an EOT (e.g. RRR or RR73) that expects a reply
    static void timerEvent(Timer *timer);  // Timer's callback function
    void startTimer(void);                 // Start timeout Timer
    void stopTimer(void);                  // Stop timeout Timer

    // Define the actions taken by the Sequencing State Machine
    void pendXmit(unsigned oddEven, SequencerStateType newState);  // Start transmitter in next timeslot

    // Helper methods
    bool isMsgForUs(Decode *msg);              // Determines if received msg is of interest to us
    Decode *getDecodedMsg(unsigned msgIndex);  // Retrieves pointer to new_decoded[] message
    void endQSO(void);                          //Terminate a QSO

    // Private member variables
    SequencerStateType state;      // The Sequencer's current state
    unsigned long sequenceNumber;  // The current timeslot's sequence number
    Timer *timeoutTimer;           // Terminates run-on transmissions after timeout period

   public:
    // Define the events triggering the Sequencing State Machine transitions
    void begin(unsigned timeoutMinutes);    // Reset sequencer
    void timeslotEvent(void);               // FT8 timeslot boundary
    void receivedMsgEvent(Decode *msg);     // Received an FT8 message
    void cqButtonEvent(void);               // CQ button clicked
    void tuneButtonEvent(void);             // TUNE button clicked
    void msgClickEvent(unsigned msgIndex);  // Received message clicked
    void abortEvent(void);                  // Abort transmission request
    void timeoutEvent(void);                // Timeout (QSO taking too long)

    // Expose getters for debugging Sequencer problems
    unsigned long getSequenceNumber(void);
    SequencerStateType getState(void);

    // Get a reference to the Sequencer singleton
    static Sequencer &getSequencer() {
        static Sequencer instance;  // Build the one-and-only Sequencer
        return instance;            // And return a reference to it
    }
};
