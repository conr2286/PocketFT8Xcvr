#include "msgTypes.h"
/**
 * SYNOPSIS
 *  Sequencer --- Implements a state machine for automated sequencing of FT8 QSOs
 *
 * NOTES
 *  Following the progress of a state machine can be challenging.
 *  While a complete FT8 implementation deals with arcane corner cases
 *  handling unexpected, exceptional events, the main flow is not so bad.
 *  Here's the main flow for Sequencer initiating a QSO by calling CQ:
 *
 *  Timeslot  CurrentState  Event           Action              NextState   Commentary
 *    0       IDLE          timeslotEvent   N/A                 IDLE        We are monitoring FT8 traffic
 *    0       IDLE          cqButtonEvent   Prepare CQ msg      CQ_PENDING  Operator pressed CQ button
 *    1       CQ_PENDING    timeslotEvent   actionXMIT          XMIT_CQ     Arm the transmitter for CQ
 *    1       XMIT_CQ       timeslotEvent   N/A                 LISTEN_LOC  Transmit CQ
 *    2       LISTEN_LOC    locatorEvent    Prepare RSL msg     RSL_PENDING Receive grid locator
 *    2       RSL_PENDING   timeslotEvent   actionXmit          XMIT_RSL    Arm transmitter for RSL
 *    3       XMIT_RSL      timeslotEvent   N/A                 LISTEN_RRSL Transmit RSL
 *    3       LISTEN_RRSL   rslEvent        Prepare RRR msg     RRR_PENDING Receive RRSL
 *    4       RRR_PENDING   timeslotEvent   actionXmit          XMIT_RRR    Arm transmitter for RRR
 *    4       XMIT_RRR      timeslotEvent   N/A                 LISTEN_73   Transmit RRR
 *    5       LISTEN_73     eotEvent        N/A                 IDLE        QSO finished normally
 *
 *  In the above, note events arising from three sources:  timeslot boundaries, GUI buttons,
 *  and received messages.  For many given events, Sequencer undertakes one of two actions:
 *  prepare a message to transmit, or arm the transmitter to begin in an appropriate timeslot.
 *  The reference [1] below provides more information about sequencing a QSO.
 *
 *  All FT8 messages, received and transmitted, worldwide, start on 15-second
 *  timeslot intervals.  The intervals begin precisely at 0, 15, 30 and 45 seconds
 *  past a minute boundary.  Each message is of a fixed bit length, and requires
 *  12.6 seconds to transmit, the remaining dwell time is available for decoding
 *  messages and preparing a response, if any.  Consider what happens in the
 *  following sequence adopted from [1] below (Sequencer's state, event and action
 *  names are ommitted to simpify the discussion):
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
 *  in odd-numbered timeslots.  K9AN replies during an odd-numbered timeslot to avoid
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
 *  signal report lost in timeslot 2.  In general, if nothing is heard when listening
 *  for a remote station to respond to our previous transmission, we retransmit the
 *  previous message.
 *
 *  For the purpose of logging, Sequencer considers a QSO successfully completed when it
 *  receives any form of end-of-transmission (EOT), e.g. RRR, RR73, or 73, from the
 *  remote station ("It aint over till it's over").  Note this is a bit different from
 *  some contests and/or special events.
 *
 *  Unexpected FT8 messages, those deviating from the so-called "standard" QSO sequence
 *  described in [1], present Sequencer with a quandry re. what to do as the documented
 *  protocol doesn't really prescribe what to do (i.e. when compared to, say, the TCP
 *  RFCs:).  The easiest thing to do would likely be abandon the QSO, but
 *  Sequencer attempts to get things back on track below.  It's confusing.
 *
 * DESIGN
 *  Sequencer implements a state machine, driven by event notification methods,
 *  and responds by invoking action methods.
 *
 *  Sequencer expects to be notified about received messages in the timeslot in which they
 *  are decoded (so it can prepare a reply to transmit in the following timeslot).
 *  If this doesn't happen for some reason, sequencer is aware of even/odd timeslots and
 *  will pend its response until a suitable timeslot when the remote station should be
 *  listening.  Things can get really fouled up beyond all recognition (FUBAR) if the
 *  remote station changes their even/odd timeslot for transmissions.
 *
 *  Sequencer employs a timer to recognize QSO wreckage (changing band conditions,
 *  remote station QRT, whatever) and abort its automated activities.
 *
 * REFERENCES
 *  1. Franke, Somerville and Taylor.  "The FT4 and FT8 Communication Protocols."
 *  QEX.  July/August 2020.
 *
 **/

#include <string.h>

#include "DEBUG.h"
#include "Sequencer.h"
#include "SequencerStates.h"
#include "button.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "traffic_manager.h"

// Many externals in the legacy C code.  TODO:  See if we can simplify these externals.
extern int Transmit_Armned;        // Transmit message pending in next timeslot
extern char Station_Call[];        // Our station's callsign
extern char Target_Call[];         // Displayed station's callsign
extern int Target_RSL;             // Remote station's RSL
extern uint16_t currentFrequency;  // Nominal frequency (sans offset) in kHz

// Macro used to determine if a timeslot sequence number is odd (1) or even (0)
// The macro evaluates to 1 if the argument is odd, else 0 if even.
#define ODD(n) (n % 2)

/**
 *  Timeslot event
 *
 *  @param sequenceNumber This timeslot's sequence number
 *
 *  The FT8 timeslot synchronizer(s) notify this event handler when a new timeslot
 *  is about to begins (we get notified before loop() takes any actions).  The
 *  Sequencer uses the timeslot's sequenceNumber to determine whether
 *  transmissions to a remote station should occur in even or odd numbered timeslots.
 *
 **/
void Sequencer::timeslotEvent() {
    DPRINTF("%s sequenceNumber=%lu, state=%u\n", __FUNCTION__, sequenceNumber, state);

    // Sequencer state determines what action to take
    switch (state) {
        // Time to start the transmitter sending previously prepared Tx6 CQ message
        case CQ_PENDING:
            DTRACE();
            actionXmit(ODD(sequenceNumber), XMIT_CQ);  // Always transmit in next timeslot
            break;

        // Time to listen for replies to our Tx6 CQ transmission.  The loop()
        // has already stopped transmitting symbols and switched us to receive mode.
        // Our job is to remember what's happening in case we hear a response to CQ.
        case XMIT_CQ:
            DTRACE();
            state = LISTEN_LOC;  // We are now listening for a response to our CQ
            break;

        // We have listened for responders to our CQ and heard nothing.  At the next timeslot,
        // we'll retransmit our previously prepared CQ message.
        // TODO:  Limit retransmissions with a timer or whatever.
        case LISTEN_LOC:
            DTRACE();
            actionXmit(ODD(sequenceNumber), XMIT_CQ);  // Transmit in next timeslot
            break;

        // We have heard a Tx1 response to our Tx6 CQ, prepared an RSL reply message, and will transmit
        // that RSL reply in the next appropriate timeslot.
        // Note:  qso records whether the responder transmits in an even or odd timeslot.  Our
        // sequenceNumber indicates the current timeslot's sequence number, not the timeslot that
        // is about to begin (we increment timeslot below).  So... if the currentSequence number
        // is even and our responder transmits in even, then we will transmit in the next (odd)
        // about-to-start timeslot when responder will be listening.
        case RSL_PENDING:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_RSL);  // Arms transmitter if next timeslot is appropriate
            break;

        // We have transmitted their RSL reply to their remote station.  Now we expect to
        // hear them send our RRSL signal report to us.
        case XMIT_RSL:
            DTRACE();
            state = LISTEN_RRSL;  // We are expecting to receive our signal report from remote station
            break;

        // After transmitting Tx2 RSL to the remote station, we have not heard a Tx3 RRSL response.
        // We will retransmit when the odd/even timeslot expects remote to be listening for us.
        case LISTEN_RRSL:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_RSL);
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit our prepared Tx4 RRR to remote station
        case RRR_PENDING:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_RRR);  // Arms the transmitter if next timeslot is appropriate
            break;

        // We have transmitted our Tx4 RRR to the remote station.  The QSO is complete as we expect to receive
        // nothing else but their 73.  We could log the QSO either here or only after we receive their 73.
        case XMIT_RRR:
            DTRACE();
            state = LISTEN_73;  // Listen for their 73
            break;

        // We listened for the remote station's 73 but have heard nothing.
        // We'll assume the QSO has ended.  TODO:  log???
        case LISTEN_73:
            DTRACE();
            state = IDLE;  // Return to idle state
            break;

        // We are waiting to contact a displayed station after operator clicked on their message.  We
        // expect msgClickEvent() to have previously initialized the QSO struct and prepared the
        // outbound message containing our own location.
        case LOC_PENDING:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_LOC);  // QSO struct was initialized by msgClickEvent()
            break;

        // We have transmitted our location to the remote station
        case XMIT_LOC:
            state = LISTEN_RSL;  // We will listen for our RSL from the remote station
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit an RRSL message to remote station
        case RRSL_PENDING:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_RRSL);  // Arm the transmitter
            break;

        // We have transmitted their RRSL message to the remote station
        case XMIT_RRSL:
            DTRACE();
            state = LISTEN_RRR;  // We expect an RRR or RR73
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit a 73 message to remote station
        case M73_PENDING:
            DTRACE();
            actionXmit(qso.oddEven, XMIT_73);  // Arm the transmitter
            break;

        // We have transmitted the 73 message to the remote station
        case XMIT_73:
            DTRACE();
            state = IDLE;  // We have finished
            // TODO:  need to log?  futz with receiver?  anything?
            break;

        // Default --- Ignore timeslot event (nothing for us to do)
        default:
            DTRACE();
            break;
    }

    // Always increment the sequenceNumber to coordinate QSO transmissions in their odd/even timeslot
    sequenceNumber++;

}  // timeSlotEvent()

/**
 *  [Re]Initialize the sequencer
 *
 *  The sequencer notes which timeslot, even or odd-numbered, in which a remote station
 *  is transmitting and attempts to avoid "doubling" with it.  This method resets the
 *  timeslot counter.
 *
 **/
void Sequencer::begin(void) {
    sequenceNumber = 0;  // Reset timeslot counter
    state = IDLE;        // Reset state to idle
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
 *  For now, display_messages() is responsible for displaying the received messages, while
 *  Sequencer assumes responsibility for checking for and responding to callers.
 *
 *  Received messages, as incoming from decode_ft8(), are too broad in scope for us to
 *  act upon until we analyze their type and trigger the appropriate event for that msgType.
 *
 **/
void Sequencer::receivedMsgEvent(Decode* msg) {
    // Sadly, some encoders apparently transmit "RR73" as a locator rather than as an EOT.
    // We work around this by assuming RR73 means end-of-transmission, not somewhere in the Arctic.
    // TODO:  Verify that we aren't guilty of this!
    if (msg->msgType == MSG_LOC && strstr(msg->field3, "RR73")) msg->msgType = MSG_RR73;

    // When debugging, print some things from the received message
    DPRINTF("%s %s %s %s msgType=%u, seq=%lu state=%u\n", __FUNCTION__, msg->field1, msg->field2, msg->field3, msg->msgType, sequenceNumber, state);

    // Stamp all received messages with the timeslot's sequenceNumber in which they were received.
    // We use this if the operator later decides to contact (click on) a displayed message.
    msg->sequenceNumber = sequenceNumber;

    // The Sequencer analyzes mesages of interest to our station
    if (isMsgForUs(msg)) {
        DPRINTF("this msg is for us:  '%s' '%s' '%s'\n", msg->field1, msg->field2, msg->field3);

        switch (msg->msgType) {
            // Did we receive another station's Tx6 CQ?
            case MSG_CQ:
                DTRACE();
                // TODO:  RoboOp
                break;

            // Did we receive their Tx1 locator message?
            case MSG_LOC:
                DTRACE();
                locatorEvent(msg);  // They are responding to us with a locator
                break;

            // Did we receive a Tx2 or Tx3 RSL containing their report of our signal?
            case MSG_RSL:
                DTRACE();
                rslEvent(msg);  // They sent our signal report
                break;

            // Did we receive an RRSL?
            case MSG_RRSL:
                DTRACE();
                rslEvent(msg);  // They sent our signal report
                break;

            // Did we receive an EOT that does not expect a reply?
            case MSG_73:
                DTRACE();
                eotEvent(msg);
                break;

            // Did we receive an EOT that expects a reply?
            case MSG_RR73:
            case MSG_RRR:
                DTRACE();
                eotReplyEvent(msg);
                break;

            // The Sequencer does not currently process certain message types
            case MSG_BLANK:
            case MSG_FREE:  // TODO:  we should try to handle this one
            case MSG_TELE:
            case MSG_UNKNOWN:
            default:
                DTRACE();
                break;
        }  // switch
    }

    DTRACE();
}

/**
 *  CQ Button clicked event
 *
 *  This event handler is invoked when the operator clicks on the GUI CQ button
 *
 **/
void Sequencer::cqButtonEvent() {
    DTRACE();

    // The required action depends upon which state the Synchronizer machine currently resides
    switch (state) {
        // Prepare to transmit CQ in the next available timeslot.  Since we are not interacting with
        // another station, we are not concerned about doubling with them, just await next timeslot.
        case IDLE:         // We are currently idle
        case LOC_PENDING:  // Operator decided to CQ rather than respond to a known station
            DTRACE();
            set_message(MSG_CQ);  // Encode our CQ message
            state = CQ_PENDING;   // Await the next timeslot
            break;

        // Abort CQ transmission (even if it's in progress)
        case CQ_PENDING:  // Pending timeslot to transmit CQ
        case XMIT_CQ:     // CQ transmission in progress
        case LISTEN_LOC:  // Listening for a response to our CQ message[s]
            DTRACE();
            state = IDLE;  // Return to idle
            terminate_transmit_armed();
            clear_FT8_message();  // Clears displayed outbound message as we're now idle
            break;

            // The CQ button is ignored during most states
        default:
            DTRACE();
            break;
    }
}

/**
 *  Click on displayed message to initiate a QSO
 *
 *  This event handler initiates a QSO by contacting a displayed, received
 *  message.
 *
 *  msgClickEvent() is invoked only by display_selected_call() after initializing
 *  the Target_Call extern with the remote station's callsign.
 *
 *  We create a QSO contact here, perhaps prematurely (Target_Call may not reply),
 *  because we need an even/odd timeslot record of when Target_Call is transmitting.
 *  Also... this may be the only time when we'll have Target_Call's locator.
 *
 **/
void Sequencer::msgClickEvent(unsigned msgIndex) {
    // Find the decoded message as we need some info from it
    Decode* msg = getDecodedMsg(msgIndex);

    // Assert Target_Call==msg->field2 as this stuff could become FUBAR
    DPRINTF("Target_Call='%s', msg->field2='%s', msg->sequenceNumber=%u\n", Target_Call, msg->field2, msg->sequenceNumber);

    // The required action depends upon which state the Synchronizer machine currently resides
    if (msg != NULL) {
        switch (state) {
            // Operator wants to send our grid locator to Target_Call
            case IDLE:        // We are currently idle
            case CQ_PENDING:  // Operator decided to contact a displayed station rather than call CQ
                DTRACE();
                qso.begin(msg->field2, currentFrequency, "FT8", ODD(msg->sequenceNumber));  // Start gathering QSO info
                qso.setWorkedLocator(msg->field3);                                          // Record their locator if we have it
                setXmitParams(msg->field2, msg->snr);                                       // Inform gen_ft8 of remote station's info
                set_message(MSG_LOC);                                                       // We initiate QSO by sending our locator
                state = LOC_PENDING;                                                        // Await appropriate timeslot to transmit to Target_Call
                break;

                // Message clicks are ignored during most states
            default:
                DTRACE();
                break;
        }
    }

}  // msgClickEvent()

/**
 * Remote station sent our signal report to us
 *
 * @param msg Their decoded RSL message
 *
 *
 **/
void Sequencer::rslEvent(Decode* msg) {
    // Action to be taken depends upon the Sequencer's current state
    switch (state) {
        // Are we expecting an RRSL?  TODO:  recvd out of sequence --> no qso
        case LISTEN_RRSL:
            DTRACE();
            qso.setMyRSL(msg->field3);             // Record our signal report in QSO
            setXmitParams(msg->field2, msg->snr);  // Inform gen_ft8 of remote station's info
            set_message(MSG_RRR);                  // Prepare an RRR message
            state = RRR_PENDING;                   // We must await an appropriate even/odd timeslot
            break;

        // Remote station sent our RSL and we should respond with their RRSL
        case LISTEN_RSL:
            DTRACE();
            qso.setMyRSL(msg->field3);             // Record our signal report in QSO structure
            setXmitParams(msg->field2, msg->snr);  // Inform gen_ft8 of remote station's info
            set_message(MSG_RRSL);                 // Prepare to send their signal report to remote station
            state = RRSL_PENDING;                  // Await an appropriate even/odd timeslot
            break;

        // We were expecting a LOC but received a signal report.  We likely were calling CQ
        // and listening for a response which came with an RSL.  Let's try to cobble-up a
        // QSO from what we have.  TODO:  may need to init the gen_ft8 externs.
        case LISTEN_LOC:
            DTRACE();
            DPRINTF("freq=%u\n", currentFrequency);
            qso.begin(msg->field2, currentFrequency, "FT8", ODD(msg->sequenceNumber));  // Begin a QSO with their station
            qso.setMyRSL(msg->field3);                                                  // Record our RSL from remote station
            setXmitParams(msg->field2, msg->snr);                                       // Inform gen_ft8 of remote station's info
            DPRINTF("Target_Call='%s', msg.field2='%s', msg.rsl=%d, Target_RSL=%d\n", Target_Call, msg->field2, msg->snr, Target_RSL);
            set_message(MSG_RSL);  // Reply with their RSL as we likely haven't sent it to them
            state = RSL_PENDING;   // Transmit their RSL in next appropriate timeslot
            break;

        // Ignore non-sense
        default:
            DTRACE();
            break;

    }  // switch

}  // rslEvent()

/**
 * Received an EOT message that does not expect a reply
 *
 * @param msg Their decoded EOT message
 *
 *
 **/
void Sequencer::eotEvent(Decode* msg) {
    // The action taken depends upon our state
    switch (state) {
        // Process a QSO ending more or less normally
        case LISTEN_73:   // We expected their 73
        case LISTEN_RRR:  // We expected an RRR but received a 73 --- I guess it's over
            DTRACE();
            state = IDLE;  // It's definitely over now
            // TODO:  Do we need to logit,  futz with GUI, flags, something else?
            break;

        // We were trying to get a CQ transmission underway and can ignore this 73 which
        // presumably is left-over from a previous QSO.
        case CQ_PENDING:
        case XMIT_CQ:
        case LISTEN_LOC:
            DTRACE();
            break;

        // We were somewhere in midst of a QSO when the remote station 73'd.  QSO incomplete.
        // TODO:  log???
        default:
            DTRACE();
            terminate_transmit_armed();  // Dis-arm the transmitter
            clear_FT8_message();         // Clears displayed outbound message
            state = IDLE;                // Return to IDLE
            break;
    }
}

/**
 * Received an EOT message that expects our 73 reply
 *
 * @param msg Their decoded EOT message
 *
 * Their RRR and RR73 messages expect our 73 reply
 *
 * We listen either for a 73 or an RRR, never an RR73.  If they send
 * us an RR73 then we treat it like an RRR and transmit a 73 reply.
 *
 **/
void Sequencer::eotReplyEvent(Decode* msg) {
    // The action taken depends upon our state
    switch (state) {
        // Process end of QSO when they expect a 73 reply from us
        case LISTEN_73:   // We expected 73 but they've requested a 73 reply.  Hmm.
        case LISTEN_RRR:  // They expect 73 from us
            DTRACE();
            setXmitParams(msg->field2, msg->snr);  // Inform gen_ft8 of remote station's info
            set_message(MSG_73);                   // Prepare an RRR message
            state = M73_PENDING;                   // We must await an appropriate even/odd timeslot
            // TODO:  qso.end()
            break;

        // We were trying to get a CQ transmission underway and can ignore the EOT which
        // presumably is left-over from a previous QSO.
        case CQ_PENDING:
        case XMIT_CQ:
        case LISTEN_LOC:
            DTRACE();
            break;

        // We were somewhere in a QSO when the remote station 73'd.  QSO incomplete.
        // TODO:  log???
        default:
            DTRACE();
            terminate_transmit_armed();  // Dis-arm the transmitter
            clear_FT8_message();         // Clears displayed outbound message
            state = IDLE;                // Return to IDLE
            break;
    }
}

/**
 *  Remote station sent us a locator (i.e. maidenhead grid square)
 *
 *  @param msg Their decoded message
 *
 *  A remote may send us a locator in response to our own CQ or perhaps to
 *  tailend our previous QSO.  The locator may be a retransmission arising
 *  after we failed to decode the original.
 *
 *  Upon entry here, sequenceNumber identifies the timeslot when the
 *  locator message was transmitted.  We record whether this timeslot is odd
 *  or even-numbered, and prepare an RSL message to transmit in the next
 *  appropriate odd/even timeslot.  If the remote station transmitted
 *  in an odd-numbered timeslot, then we plan to transmit in an even-
 *  numbered timeslot.
 *
 **/
void Sequencer::locatorEvent(Decode* msg) {
    // The action to be taken depends upon the Sequencer's current state
    switch (state) {
        // We have a CQ pending or were listening for a response to our own CQ
        case CQ_PENDING:  // We were going to [re]transmit CQ but received a call
        case LISTEN_LOC:  // We were listening for a response to our CQ
            DTRACE();
            qso.begin(msg->field2, currentFrequency, "FT8", ODD(sequenceNumber));  // Start gathering QSO info
            qso.setWorkedLocator(msg->field3);                                     // Record responder's locator
            setXmitParams(msg->field2, msg->snr);                                  // Inform gen_ft8 of remote station's info
            set_message(MSG_RSL);                                                  // Prepare to transmit RSL to responder
            state = RSL_PENDING;                                                   // Must await an appropriate timeslot when responder is listening
            break;

            // TODO:  qso.end();

        // Something is wrong and we don't know why or how we apparently received their locator
        default:
            DTRACE();
            break;

    }  // switch

}  // locatorEvent()

/**
 *  Determine if a received message is of concern to us
 *
 *  @param msg Pointer to the decoded message
 *
 *  @return true if interesting, false if not
 *
 *  Examples of messages [not] of interest to KQ7B:
 *
 *  CQ AG0E EN15          //Interesting
 *  KQ7B AG0E -9          //Interesting
 *  CQ POTA AG0E EN15     //Not interesting (TODO someday)
 *. KQ7B/R AG0E EN15      //Interesting
 *  F4CQS KX8XX EN74      //Not interesting
 *. KQ7BA AG0E EN15       //Not interesting
 *
 **/
bool Sequencer::isMsgForUs(Decode* msg) {
    // char dstCallSign[13];
    // char *p1, *p2;

    // Trim the destination callsign for use by the Sequencer.  Examples of supported callsigns
    // include:  KQ7B, KQ7B/R, CQ, CQ POTA and these become KQ7B, KQ7B/R, and CQ.
    //  p1 = msg->field1;
    //  p2 = dstCallSign;
    //  while ((*p1 != 0) && (*p1 == ' ')) p1++;           //Scan past leading spaces
    //  while ((*p1 != 0) && (*p1 != ' ')) *p2++ = *p1++;  //Copy callsign
    //  *p2 = 0;                                           //Append a NUL to callsign
    // DPRINTF("dstCallSign='%s', Station_Call='%s'\n", dstCallSign, Station_Call);

    // A received msg is "for us" if our callsign or CQ appears as the destination station's callsign
    bool myCall = strncmp(msg->field1, Station_Call, sizeof(msg->field1)) == 0;  // Sent directly to us?
    bool cq = strncmp(msg->field1, "CQ", sizeof(msg->field1)) == 0;              // CQ
    bool msgIsForUs = cq || myCall;
    // DPRINTF("isMsgForUs()=%u %s\n",msgIsForUs,msg->field1);
    return msgIsForUs;
}

/**
 *  Action routine to begin modulation in the requested odd/even timeslot
 *
 *  @param oddEven begin modulation in 1==odd, 0==even-numbered timeslot
 *  @param s new state value if transmitter is armed
 *
 *  We don't actually modulate anything here, all we do is setup the flags so loop()
 *  will begin transmitting FSK modulated symbols.
 *
 *  + You must invoke set_message() to prepare the outbound message prior
 *  to invoking actionXmit().
 *
 *  + Upon entry, sequenceNumber identifies the current timeslot and, if we
 *  arm the transmitter, loop() will begin modulation in the *following*
 *  timeslot.  Thus, to begin modulation in an odd-numbered timeslot, we
 *  arm the transmitter in an even-numbered timeslot.  :|
 *
 *  We do nothing if the next timeslot is not appropriate.
 *
 **/
void Sequencer::actionXmit(unsigned oddEven, SequencerStateType newState) {
    oddEven &= 0x01;  // Force binary value:  1==odd, 0==even-numbered timeslot

    DPRINTF("oddEven=%u, sequenceNumber=%lu newState=%u\n", oddEven, sequenceNumber, newState);

    // Do we want to transmit in the *next* timeslot?
    if (oddEven == ODD(sequenceNumber)) {
        DTRACE();
        Transmit_Armned = 1;                   // Yes, transmit in next slot
        setup_to_transmit_on_next_DSP_Flag();  // loop() begins modulation in next timeslot
        state = newState;                      // Advance state machine to new state after arming the transmitter
    }

}  // actionXmit()






/**
 *  Helper routine to retrieve pointer to a decoded message
 *
 *  @param msgIndex Index of decoded message
 *
 *  @return pointer to Decode entry or NULL if msgIndex invalid
 *
 **/
extern Decode new_decoded[];  // References decode_ft8.cpp defining 20 elements
Decode* Sequencer::getDecodedMsg(unsigned msgIndex) {
    if (msgIndex < 20)
        return &new_decoded[msgIndex];
    else
        return NULL;
}





/**
 * Getter for debugging sequenceNumber problems
 * 
 * @return sequenceNumber
 * 
 */
unsigned long Sequencer::getSequenceNumber() {
    return sequenceNumber;
}