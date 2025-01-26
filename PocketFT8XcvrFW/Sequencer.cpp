/**
 * SYNOPSIS
 *  Sequencing --- Implements a state machine for sequencing QSOs begun with our CQ or call
 *
 * NOTES
 *  All FT8 messages, received and transmitted, worldwide, occur at 15-second
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
 *  "doubling" with K1JT's retransmissions.  If K9AN initially decodes K1JT's CQ during
 *  timeslot 4, they had only 2.4 seconds, at most, to choose to respond in the following
 *  timeslot (5).  If they procrastinated too long, then their response would necessarily
 *  have to wait for the next odd numbered timeslot (7) as K1JT likely will retransmit CQ
 *  during timeslot 7.  
 *
 *  Proper sequencing is important, and is most effectively automated due to the short
 *  dwell times available between when a message is decoded, displayed and the following
 *  timeslot begins.  Now consider the following in which a message is lost in the noise:
 *
 *  Timeslot Destn Source  Content Commentary
 *     0      CQ    K1JT    FN20    K1JT transmits CQ
 *     1      K1JT  K9AN    EN50    K9AN responds to K1JT with their grid square locator
 *     2      K9AN  K1JT    -20     K9AN listens but K1JT's transmission is lost in the noise
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
 *  Sequencer is implemented as a state machine, driven by event notifications (methods),
 *  and responding with actions (methods).
 *
 *  Sequencer should be notified about received messages in the timeslot in which they
 *  are decoded (so it can prepare a response to transmit in the following timeslot).
 *  If this doesn't happen for some reason, sequencer is aware of even/odd timeslots and
 *  will pend its response until a suitable timeslot when the remote station should be
 *  listening.  Things can get really fouled up beyond all recognition (FUBAR) if the
 *  remote station changes their even/odd timeslot for transmissions.  
 *
 *  Sequencer employs a timer to recognize an incomplete QSO (changing band conditions,
 *  QRT remote station, whatever).
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

  this->sequenceNumber++;
  DPRINTF("sequenceNumber=%lu\n",this->sequenceNumber);

}




/**
 *  [Re]Initialize the sequencer
 *
**/
void Sequencer::begin(void) {
  this->sequenceNumber=0;       //Reset timeslot counter
  DPRINTF("Sequencer reset\n");
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
void Sequencer::receivedMsgEvent(Decode* msg) {

  //When debugging, print some things from the received message
  DPRINTF("%s %s %s msgType=%u, seq=%lu\n", msg->field1, msg->field2, msg->field3, msg->msgType, this->sequenceNumber);

}
