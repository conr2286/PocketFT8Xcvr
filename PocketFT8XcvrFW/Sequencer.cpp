/**
 * SYNOPSIS
 *  Sequencer --- Implements a state machine for sequencing QSOs begun with our CQ or call to remote
 *
 * NOTES
 *  All FT8 messages, received and transmitted, worldwide, transmit at 15-second
 *  timeslot intervals.  The intervals begin precisely at 0, 15, 30 and 45 seconds
 *  past a minute boundary.  Each message is of a fixed bit length, and requires
 *  12.6 seconds to transmit, the remaining dwell time is available for decoding
 *  messages and preparing a response, if any.  Consider the following sequence
 *  adopted from [1] below:
 *
 *  Timeslot  Destn Source  Content Commentary
 *     0      CQ    K1JT    FN20    K1JT transmits CQ
 *     1                            K1JT listens but hears no response
 *     2      CQ    K1JT    FN20    Retransmit CQ
 *     3                            K1JT again hears no response
 *     4      CQ    K1JT    FN20    Retransmit CQ
 *     5      K1JT  K9AN    EN50    K9AN responds to K1JT with their grid square locator
 *     6      K9AN  K1JT    -10     K1JT sends K9AN's signal report (dB)
 *     7      K1JT  K9AN    R-12    K9AN rogers the report and transmits K1JT's report
 *     8      K9AN  K1JT    RRR     K9AN rogers the received report
 *     9      K1JT  K9AN    73      Traditional QSO ending
 *           
 *  K1JT is [re]transmitting CQ during even-numbered timeslots and listening for responses
 *  in odd-numbered timeslots.  K9AN responds during an odd-numbered timeslot to avoid
 *  "doubling" with K1JT's transmissions.  If K9AN initially decodes K1JT's CQ during
 *  timeslot 4, they had only 2.4 seconds, at most(!), to choose to respond in the following
 *  timeslot (5).  If they procrastinated, then K9AN's response would necessarily
 *  have to wait for the next odd numbered timeslot (7) as K1JT will likely retransmit CQ
 *  during timeslot 7.  
 *
 *  Proper sequencing is important, and is most effectively automated due to the short
 *  dwell time available between when a message is decoded, displayed and the following
 *  timeslot begins.  Human operators simply can't react that fast.  Now consider the
 *  following in which K1JT's response is lost in the ozone:
 *
 *  Timeslot Destn Source  Content Commentary
 *     0      CQ    K1JT    FN20    K1JT transmits CQ
 *     1      K1JT  K9AN    EN50    K9AN responds to K1JT with their grid square locator
 *     2      K9AN  K1JT    -20     K9AN listens but K1JT's transmission is lost in noise
 *     3      K1JT  K9AN    EN50    K9AN retransmits their grid square locator
 *     4      K9AN  K1JT    -20     K1JT retransmits the lost signal report
 *     5      K1JT  K9AN    R-12    K9AN rogers the received report and transmits K1JT's report
 *     6      K9AN  K1JT    RRR     K9AN rogers the received report
 *     7      K1JT  K9AN    73      Traditional QSO ending
 *
 *  In the above, having heard nothing in timeslot 2, K9AN retransmits their grid square in
 *  timeslot 3 (when K1JT should be listening) in hope of recovering the QSO.  After
 *  successfully receiving K9AN's retransmission in timeslot 3, K1JT retransmits the
 *  signal report lost in timeslot 2.  
 *
 *  For the purpose of logging, Sequencer considers a QSO successfully completed when it
 *  receives any form of end-of-transmission (EOT), e.g. RRR, RR73, or 73, from the
 *  remote station ("It aint over till it's over").  Note this is a bit different from
 *  some contests and/or special events.
 *
 * DESIGN 
 *  Sequencer implements a state machine, driven by event notification methods,
 *  and responds by invoking action methods.
 *
 *  Sequencer should be notified about received messages in the timeslot in which they
 *  are decoded (so it can prepare a response to transmit in the following timeslot).
 *  If this doesn't happen for some reason, sequencer is aware of even/odd timeslots and
 *  will pend its response until a suitable timeslot when the remote station should be
 *  listening.  Things can get really fouled up beyond all recognition (FUBAR) if the
 *  remote station changes their even/odd timeslot for transmissions.  
 *
 *  Sequencer employs a timer to recognize an incomplete QSO (changing band conditions,
 *  remote station QRT, whatever).
 *
 * REFERENCES
 *  1. Franke, Somerville and Taylor.  "The FT4 and FT8 Communication Protocols."
 *  QEX.  July/August 2020.
 *
**/

#include "DEBUG.h"
#include "Sequencer.h"
#include "SequencerStates.h"
#include "decode_ft8.h"



/**
 *  Timeslot event
 *
 *  @param sequenceNumber This timeslot's sequence number
 *
 *  The FT8 timeslot synchronizer(s) notify this event handler when a new timeslot
 *  begins.  The Sequencer uses the timeslot's sequenceNumber to determine whether
 *  transmissions to a remote station should occur in even or odd numbered timeslots.
 *
**/
void Sequencer::timeslotEvent() {

  sequenceNumber++;
  DPRINTF("%s sequenceNumber=%lu\n", __FUNCTION__, sequenceNumber);
}




/**
 *  [Re]Initialize the sequencer
 *
 *  The sequencer notes which timeslot, even or odd-numbered, in which a remote station 
 *  is transmitting and attempts to avoid "doubling" with it.  This method resets the
 *  timeslot counter.
 *
**/
void Sequencer::begin(void) {
  sequenceNumber = 0;  //Reset timeslot counter
  state = IDLE;        //Reset state to idle
  DPRINTF("%s\n", __FUNCTION__);
}




/**
 *  Received message event
 * 
 *  @param msg Reference to the decoded message
 *
 *  The receiver notifies this event handler when it has successfully decoded a message:
 *    msg->field1   Destination station (may be our station, another station, CQ, or QRZ)
 *    msg->field2   Source station
 *    msg->field3   Conveyed content
 *    msg->msgType  Defines what to expect in field3 (e.g. LOC, RSL, 73...)
 *
**/
void Sequencer::receivedMsgEvent(unsigned index) {

  //When debugging, print some things from the received message
  DPRINTF("%s %s %s %s msgType=%u, seq=%lu\n", __FUNCTION__, msgs[index].field1, msgs[index].field2, msgs[index].field3, msgs[index].msgType, sequenceNumber);
}





/**
 *  CQ Button clicked event
 *
 *  This event handler is invoked when the operator clicks on the GUI CQ button
 *
**/
void Sequencer::cqButtonEvent() {

  DPRINTF("%s state=%u\n", __FUNCTION__, state);

  //The required action depends upon which state the Synchronizer machine currently resides
  switch (state) {

    //Prepare to transmit CQ in the next appropriate timeslot
    case IDLE:         //We are currently idle
    case LOC_PENDING:  //Operator decided to CQ rather than respond to a known station

      break;

    //The CQ button is ignored during most states
    case CQ_PENDING:   //Impatient operator --- CQ is already pending
    case XMIT_CQ:      //Impatient operator --- we are already transmitting CQ
    case LISTEN_LOC:   //Impatient operator --- we are awaiting response to our CQ
    case XMIT_RSL:     //QSO is in progress
    case LISTEN_RRSL:  //QSO is in progress
    case XMIT_RR73:    //QSO is in progress
    case LISTEN_73:    //QSO is in progress
    case XMIT_LOC:     //CALLing: Transmitting our locator in response to their CQ
    case LISTEN_RSL:   //QSOing:  Listening for our RSL
    case XMIT_RRSL:    //QSOing:  Transmitting Roger and their RSL
    case LISTEN_RRR:   //QSOing:  Listen for their RRR/RR73/73
    case XMIT_73:      //QSOing:  Transmitting 73
      break;
  }
}





/**
 *  Displayed message click event
 *
 *
**/
void Sequencer::msgClickEvent(unsigned index) {

  //When debugging, print some things from the clicked message
  DPRINTF("%s %s %s msgType=%u, seq=%lu\n", msgs[index].field1, msgs[index].field2, msgs[index].field3, msgs[index].msgType, sequenceNumber);
}
