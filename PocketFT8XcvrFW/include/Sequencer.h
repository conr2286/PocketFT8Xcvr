#pragma once

#include "AScrollBox.h"
#include "Contact.h"
#include "LogFactory.h"
#include "SequencerStates.h"
#include "Timer.h"
#include "UserInterface.h"
#include "decode_ft8.h"

void setAutoReplyToCQ(bool);
bool getAutoReplyToCQ(void);

class Sequencer {
   private:
    // Information saved about a QSO
    Contact contact;     // Info gathered about the QSO
    char theirTimeslot;  // 0==even, 1==odd

    // The Sequencer singleton's private constructor
    Sequencer() : state(IDLE), sequenceNumber(0), timeoutTimer(nullptr), contactLog(nullptr), lastStationMsgsItem(nullptr) {
    }  // Sequencer()

    // Delete copy constructor and assignment operator to prevent copying
    // Sequencer(const Sequencer &) = delete;
    // Sequencer &operator=(const Sequencer &) = delete;

    // Define the events arising from analysis of received messages
    void locatorEvent(Decode *msg);   // Received their maidenhead locator in msg
    void rslMsgEvent(Decode *msg);       // Received our signal report in msg
    void eotMsgNoReplyEvent(Decode *msg);       // Received an EOT (e.g. 73) that doesn't expect a reply
    void eotMsgReplyEvent(Decode *msg);  // Received an EOT (e.g. RRR or RR73) that expects a reply
    void cqMsgEvent(Decode *msg);     // Received a non-directed CQ

    // Other internally-generated events
    static void onTimerEvent(Timer *timer);  // Timer's callback function
    void startTimer(void);                   // Start timeout Timer
    void stopTimer(void);                    // Stop timeout Timer


    // Define the actions taken by the Sequencing State Machine
    void actionPendXmit(unsigned oddEven, SequencerStateType newState);  // Start transmitter in next timeslot

    // Helper methods
    bool isMsgForUs(Decode *msg);              // Determines if received msg is of interest to us
    Decode *getDecodedMsg(unsigned msgIndex);  // Retrieves pointer to new_decoded[] message
    void endQSO(void);                         // Terminate a QSO

    // Private member variables
    SequencerStateType state;             // The Sequencer's current state
    unsigned long sequenceNumber;         // The current timeslot's sequence number
    Timer *timeoutTimer;                  // Terminates run-on transmissions after timeout period
    ContactLogFile *contactLog;           // The contact log file
    String lastReceivedMsg;               // The last received (decoded) message text
    String lastTransmittedMsg;            // The last transmitted message text
    AScrollBoxItem *lastStationMsgsItem;  // Pointer to last item in StationMsgs box

    // Misc helpers
    void highlightAbortedTransmission(void);  // in Station Messages

   public:
    // Define the events triggering the Sequencing State Machine transitions
    void begin(unsigned timeoutMinutes, const char *logfileName);  // Reset sequencer
    void timeslotEvent(void);                                      // FT8 timeslot boundary
    void receivedMsgEvent(Decode *msg);                            // Received an FT8 message
    void cqButtonEvent(void);                                      // CQ button clicked
    void msgButtonEvent(char *freeTxtMsg);                         // Send a 13 char free txt msg button
    void abortButtonEvent(void);                                   // Operator clicked ABORT button
    void tuneButtonEvent(void);                                    // TUNE button clicked
    void clickDecodedMessageEvent(unsigned msgIndex);              // Received message clicked this index
    void clickDecodedMessageEvent(Decode *msg);                    // Received messages clicked this decoded msg
    

    // void abortEvent(void);                  // Abort transmission request
    static void onTimerEvent(void);  // Timeout (QSO taking too long)

    // Expose getters for debugging Sequencer problems
    unsigned long getSequenceNumber(void);
    SequencerStateType getState(void);

    // Get a reference to the Sequencer singleton
    static Sequencer &getSequencer() {
        static Sequencer instance;  // Build the one-and-only Sequencer
        return instance;            // And return a reference to it
    }
};
