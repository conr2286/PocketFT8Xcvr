#pragma once

#include "decode_ft8.h"
#include "SequencerStates.h"


class Sequencer {

private:

  //The Sequencer singleton's private constructor
  Sequencer() {
    sequenceNumber = 0;
    msgs = getNewDecoded();
  }

  // Delete copy constructor and assignment operator to prevent copying
  // Sequencer(const Sequencer &) = delete;
  // Sequencer &operator=(const Sequencer &) = delete;

  //Define the events arising from analysis of received messages
  void locatorEvent(Decode *msg);  //Received their maidenhead locator in msg
  void rslEvent(Decode *msg);      //Received our signal report in msg
  void eotEvent(Decode *msg);      //Received any form of EOT (e.g. RRR, RR73 or 73) msg


  //Define the actions taken by the Sequencing State Machine
  void actionXmitCQ();        //Transmit our CQ message
  void actionXmitRSL(/*?*/);  //Transmit their signal report
  void actionXmitLoc(/*?*/);  //Transmit our maidenhead locator
  void actionXmitR73(/*?*/);  //Transmit 73
  void actionLogQSO(/*?*/);   //Log completed QSO
  void actionAbort(void);     //Abort QSO, transmission or tuning, and return to idle

  //Private member variables
  SequencerStateType state;      //The Sequencer's current state
  unsigned long sequenceNumber;  //The current timeslot's sequence number
  Decode *msgs;                  //Decoded messages


public:

  //Define the events triggering the Sequencing State Machine transitions
  void begin();                           //Reset sequencer
  void timeslotEvent(void);               //FT8 timeslot boundary
  void receivedMsgEvent(unsigned index);  //Received an FT8 message
  void cqButtonEvent(void);               //CQ button clicked
  void msgClickEvent(unsigned index);     //Received message clicked
  void abortEvent(void);                  //Abort transmission request
  void timeoutEvent(void);                //Timeout (QSO taking too long)

  //Get a reference to the Sequencer singleton
  static Sequencer &getSequencer() {
    static Sequencer instance;  //Build the one-and-only Sequencer
    return instance;            //And return a reference to it
  }
};
