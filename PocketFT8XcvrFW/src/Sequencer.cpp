/**
 * SYNOPSIS
 *  Sequencer --- (AKA RoboOp) Implements a state machine for automated sequencing of FT8 QSOs
 *
 * !!!DON'T PANIC!!!
 *  Following the progress of a large state machine like this is challenging.
 *  While the complete FT8 implementation accommodates arcane corner cases to
 *  handle unexpected, exceptional events, the main flow is not so bad.
 *  Here's the main flow for how the Sequencer initiates a QSO by calling CQ:
 *
 *  Timeslot  CurrentState  Event           Action              NextState   Commentary
 *    0       IDLE          timeslotEvent   N/A                 IDLE        We are monitoring FT8 traffic
 *    0       IDLE          cqButtonEvent   Prepare CQ msg      CQ_PENDING  Operator pressed CQ button
 *    1       CQ_PENDING    timeslotEvent   pendXmit
 *            XMIT_CQ       Arm the transmitter for CQ
 *    1       XMIT_CQ       timeslotEvent   N/A                 LISTEN_LOC  Transmit CQ
 *    2       LISTEN_LOC    locatorEvent    Prepare RSL msg     RSL_PENDING Receive grid locator
 *    2       RSL_PENDING   timeslotEvent   pendXmit
 *            XMIT_RSL      Arm transmitter for RSL
 *    3       XMIT_RSL      timeslotEvent   N/A                 LISTEN_RRSL Transmit RSL
 *    3       LISTEN_RRSL   rslEvent        Prepare RRR msg     RRR_PENDING Receive RRSL
 *    4       RRR_PENDING   timeslotEvent   pendXmit
 *            XMIT_RRR      Arm transmitter for RRR
 *    4       XMIT_RRR      timeslotEvent   N/A                 LISTEN_73   Transmit RRR
 *    5       LISTEN_73     eotEvent        N/A                 IDLE        QSO finished normally
 *
 *  The above includes events arising from three sources:  timeslot boundaries, GUI buttons,
 *  and received messages.  For many events, Sequencer undertakes one of two actions:
 *  prepare a message to transmit, or arm the transmitter to begin in an appropriate timeslot.
 *  The reference [1] below provides more information about sequencing an FT8 QSO.
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
 *  timeslot 4, they had 2.4 seconds, at most(!), to choose to respond in the following
 *  timeslot (5).  If they procrastinated, then K9AN's response would necessarily
 *  have to wait for the next odd numbered timeslot (7) as K1JT will likely retransmit CQ
 *  during timeslot 7.
 *
 *  Proper sequencing is important, and is most effectively automated due to the short
 *  dwell time available between when a message is decoded, displayed and the following
 *  timeslot begins.  Human operators simply can't prepare a message in time to send
 *  in that next timeslot --- that's why we have RoboOp (if you've ever worked a CW
 *  contest at 02:00 local time then you know the job).  Now consider the following in
 *  which K1JT's response is lost in the ozone:
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
 *  some contests and/or special events --- we're a bit conservative here.
 *
 *  Unexpected FT8 messages, those deviating from the so-called "standard" QSO sequence
 *  described in [1], present Sequencer with a quandry re. what to do as the documented
 *  protocol doesn't really prescribe what to do (i.e. when compared to, say, the TCP
 *  RFCs:).  The easiest thing to do would likely be abandon the QSO, but
 *  Sequencer attempts to get things back on track below.  It's confusing, and much of
 *  the code below addresses the arcane situations when the QSO flows abnormally.
 *
 * DESIGN
 *  Sequencer implements a state machine, driven by event notification methods,
 *  and responds by invoking action methods.  There are two "main" forms of events,
 *  timeslots and received messages.  Others (e.g. Timers) deal with oddities.
 *  RoboOp is a lot of code, but it attempts to behave as would a (very fast) human
 *  operator.
 *
 *  Sequencer expects to be notified about received messages in the timeslot in which they
 *  are decoded (so it can prepare a response to transmit in the following timeslot).
 *  If this doesn't happen for some reason, sequencer is aware of even/odd timeslots and
 *  will pend its response until a suitable timeslot when the remote station should be
 *  listening.  The QSO can get really fouled up beyond all recognition (FUBAR) if the
 *  remote station changes their even/odd timeslot for transmissions.
 *
 *  Sequencer employs a timer to recognize QSO wreckage (changing band conditions,
 *  remote station QRT, whatever) and abort its automated activities.
 *
 * NOMENCLATURE
 *  "Events" refer to things (e.g. received messages, timeslot boundaries, menu
 *  buttons, timers...) that trigger the state machine.
 *  "Actions" refer to the activities undertaken by the state machine in response to
 *  events.
 *
 * REFERENCES
 *  1. Franke, Somerville and Taylor.  "The FT4 and FT8 Communication Protocols."
 *  QEX.  July/August 2020.
 *
 **/
#include "Sequencer.h"

#include <Arduino.h>
#include <String.h>

#include "Config.h"
#include "DEBUG.h"
#include "LogFactory.h"
#include "PocketFT8Xcvr.h"
#include "SequencerStates.h"
#include "UserInterface.h"
#include "button.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "msgTypes.h"
#include "traffic_manager.h"

// Many externals in the legacy C code.  TODO:  See if we can simplify these externals.
extern int Transmit_Armned;  // (Maybe) Transmit message pending in next timeslot
extern int xmit_flag;        // Transmitting modulated symbols
extern char Target_Call[];   // Displayed station's callsign
extern int Target_RSL;       // Remote station's RSL

// Our statics
static bool autoReplyToCQ;  // RoboOp automatically transmits reply to CQ

// Macro used to determine if a timeslot sequence number is odd (1) or even (0)
// The macro evaluates to 1 if the argument is odd, else 0 if even.
#define ODD(n) (n % 2)

/**
 * @brief [Re]Initialize the sequencer
 * @param timeoutSeconds #seconds Sequencer will retransmit msg without a response
 * @param logFileName Name of the ADIF logfile
 *
 *  The sequencer notes which timeslot, even or odd-numbered, in which a remote station
 *  is transmitting and attempts to avoid "doubling" with it.  This method resets the
 *  timeslot counter.
 *
 * The timeout parameter governs how long the Sequencer retransmits a message
 * (including CQ) without receiving a response from a remote station.
 *
 *  This method should be invoked once, probably by setup()
 *
 **/
void Sequencer::begin(unsigned timeoutSeconds, const char* logfileName) {
    DTRACE();
    sequenceNumber = 0;                                                      // Reset timeslot counter
    state = IDLE;                                                            // Reset state to idle
    timeoutTimer = Timer::buildTimer(timeoutSeconds * 1000L, onTimerEvent);  // Build the QSO/tuning timeout-timer
    contactLog = LogFactory::buildADIFlog(logfileName);
    setAutoReplyToCQ(false);                          // Disable auto reply to CQ and clear button
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);  // Display RECEIVE icon
    lastReceivedMsg = String("");                     // We haven't received anything yet
}  // begin()

/**
 *  Timeslot event
 *
 *  @param sequenceNumber This timeslot's sequence number
 *
 *  A "timeslot" refers to the 15 second FT8 intervals beginning at 0, 15, 30 and 45
 *  seconds past the minute.
 *
 *  The FT8 timeslot synchronizer notifies this event handler when a new timeslot
 *  is about to begin (we get notified before loop() takes any action in that timeslot).
 *  The Sequencer uses the timeslot's sequenceNumber to determine whether
 *  transmissions to a remote station should occur in even or odd numbered timeslots,
 *  an important consideration ensuring our transmissions do not "double" with those
 *  of a remote station in our QSO.
 *
 **/
void Sequencer::timeslotEvent() {
    DPRINTF("%s sequenceNumber=%lu, state=%u\n", __FUNCTION__, sequenceNumber, state);

    // Review and purge ancient messages from UI
    ui.stationMsgs->reviewTimeStamps();  // Station messages
    ui.decodedMsgs->reviewTimeStamps();  // Decoded messages

    // Sequencer state determines what action to take
    switch (state) {
        // We are idle, monitoring the traffic --- there's nothing for Sequencer machine to do
        // Or TUNING in which case we also do nothing (Timer will eventually stop run-on Tuning)
        case TUNING:
        case IDLE:
            // DTRACE();
            break;

        // Time to start the transmitter sending previously prepared Tx6 CQ message
        case CQ_PENDING:
            DTRACE();
            pendXmit(ODD(sequenceNumber), XMIT_CQ);  // Set modulation flag for loop()
            break;

        // Time to listen for replies to our Tx6 CQ transmission.  The loop()
        // has already stopped transmitting symbols and switched us to receive mode.
        // Our job is to prepare to process a possible response to our CQ.
        case XMIT_CQ:
            DTRACE();
            state = LISTEN_LOC;  // We are now listening for a response to our CQ
            // ui.applicationMsgs->setText(get_message(), A_DARK_GREEN);
            break;

        // We have listened for responders to our CQ and heard nothing.  At the next timeslot,
        // we'll retransmit the CQ message.
        case LISTEN_LOC:
            DTRACE();
            pendXmit(ODD(sequenceNumber), XMIT_CQ);  // Arm transmitter now if needed in next timeslot
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
            pendXmit(contact.oddEven, XMIT_RSL);  // Arm transmitter now if needed in next timeslot
            break;

        // We have transmitted their RSL reply to their remote station.  Now we expect to
        // hear them send our RRSL signal report to us.  Receiver is already running.
        case XMIT_RSL:
            DTRACE();
            state = LISTEN_RRSL;  // We are expecting to receive our signal report from remote station
            break;

        // After transmitting Tx2 RSL to the remote station, we have not heard a Tx3 RRSL response.
        // We will retransmit when the odd/even timeslot expects remote to be listening for us.
        case LISTEN_RRSL:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_RSL);  // Arm transmitter now if needed in next timeslot
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit our prepared Tx4 RRR to remote station
        case RRR_PENDING:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_RRR);  // Arm transmitter now if needed in next timeslot
            break;

        // We have transmitted our Tx4 RRR to the remote station.  The QSO is complete as we expect to receive
        // nothing else but their 73.  We could log the QSO either here or only after we receive their 73.
        // Our receiver is already running.
        case XMIT_RRR:
            DTRACE();
            state = LISTEN_73;  // Listen for their 73
            break;

        // We listened for the remote station's 73-type msg but have heard nothing.
        // Let's retransmit our previous msg to tease-out a response from remote and
        // rely on the timer to close the QSO if it's hopeless.  Our state remains
        // unchanged while we continue to listen.
        case LISTEN_73:
        case LISTEN_RRR:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_RRSL);  // Retransmit our RRSL to remote
            break;

        // We are waiting to contact a displayed station after operator clicked on their message.  We
        // expect clickDecodedMessageEvent() to have previously initialized the QSO struct and prepared the
        // outbound message containing our own location.
        case LOC_PENDING:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_LOC);  // Arm transmitter now if needed in next timeslot
            break;

        // We have transmitted our location to the remote station and are now hopefully receiving their RSL
        case XMIT_LOC:
            DTRACE();
            state = LISTEN_RSL;  // We will listen for our RSL from the remote station
            break;

        // We were listening for remote's RSL message to us but heard nothing.  Retransmit LOC.
        case LISTEN_RSL:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_LOC);  // Arm transmitter now if needed in next timeslot
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit an RRSL message to remote station
        case RRSL_PENDING:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_RRSL);  // Arm transmitter now if needed in next timeslot
            break;

        // We have transmitted their RRSL message to the remote station and are now receiving their response
        case XMIT_RRSL:
            DTRACE();
            state = LISTEN_RRR;  // We expect an RRR or RR73
            break;

        // We are waiting for an appropriate even/odd timeslot to transmit a 73 message to remote station
        case M73_PENDING:
            DTRACE();
            pendXmit(contact.oddEven, XMIT_73);  // Arm transmitter now if needed in next timeslot
            break;

        // We have transmitted the 73 message to the remote station
        case XMIT_73:
            DTRACE();
            endQSO();
            state = IDLE;  // We have finished
            break;

        // Unexpected state !?
        default:
            DPRINTF("Unexpected Sequencer state =%u\n", state);
            break;
    }

    // Always increment the sequenceNumber to coordinate QSO transmissions in their odd/even timeslot
    sequenceNumber++;

}  // timeSlotEvent()

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
 *  In general (see exceptions below), to sustain a QSO during difficult conditions, we restart
 *  the timeout Timer when we receive a message from the remote station.  This means the
 *  Timer aborts a run-on QSO after we don't hear anything from the remote for a "long" time.
 *  There is some risk with this approach:  if the remote repeatedly retransmits the same
 *  FT8 message "forever" then we will continue to respond even though the QSO doesn't make
 *  progress.  If this proves problematic, we could use a 2nd Timer to kill a looping QSO.
 *
 **/
void Sequencer::receivedMsgEvent(Decode* msg) {
    // Sadly, some FT8 encoders apparently transmit "RR73" as a locator rather than as an EOT.
    // We work around this by assuming RR73 means end-of-transmission, not somewhere in the Arctic.
    // TODO:  Verify our encoder isn't guilty of this!
    if (msg->msgType == MSG_LOC && strstr(msg->field3, "RR73")) msg->msgType = MSG_RR73;

    // When debugging, print some things from the received message
    DPRINTF("%s %s %s %s msgType=%u, sequenceNumber=%lu state=%u\n", __FUNCTION__, msg->field1, msg->field2, msg->field3, msg->msgType, sequenceNumber, state);

    // Build a String of the received message fields for us to display
    static const String sp(" ");
    String thisReceivedMsg = String(msg->field1) + sp + String(msg->field2) + sp + String(msg->field3);

    // The Sequencer analyzes mesages of interest to our station
    if (isMsgForUs(msg)) {
        DPRINTF("this msg is for us:  '%s' '%s' '%s'\n", msg->field1, msg->field2, msg->field3);

        // Display messages sent directly to us (not a CQ) in StatoinMsgs
        if (msg->msgType != MSG_CQ) {
            // Display messages addressed to us in our Station Messages box.  Retransmissions appear
            // in yellow rather than wasting another line in our little Station Messages box
            if (thisReceivedMsg == lastReceivedMsg) {
                ui.stationMsgs->setItemColors(lastStationMsgsItem, A_YELLOW, A_BLACK);  // Recolor previous (retransmitted) msg
            } else {
                lastStationMsgsItem = ui.stationMsgs->addStationMessageItem(ui.stationMsgs, thisReceivedMsg);  // New received msg
            }
            lastReceivedMsg = thisReceivedMsg;  // Remember this received msg text for when the next msg arrives
        }

        switch (msg->msgType) {
            // Did we receive a station's Tx6 CQ?
            case MSG_CQ:
                DTRACE();
                cqMsgEvent(msg);
                break;

            // Did we receive their Tx1 locator message?
            case MSG_LOC:
                DTRACE();
                startTimer();       // Keep this QSO alive as long as remote station is responding
                locatorEvent(msg);  // They are responding to us with a locator
                break;

            // Did we receive a [R]RSL containing our signal report?
            case MSG_RSL:
            case MSG_RRSL:
                DTRACE();
                startTimer();   // Keep this QSO alive while remote station continues to respond
                rslEvent(msg);  // They sent our signal report
                break;

            // Did we receive an EOT that does not expect a reply?  We don't restart the Timer for EOT.
            case MSG_73:
                DTRACE();
                eotEvent(msg);  // No reply to 73 msg
                break;

            // Did we receive an EOT that expects a reply?  We don't restart the Timer for EOT.
            case MSG_RR73:
            case MSG_RRR:
                DTRACE();
                eotReplyEvent(msg);  // We reply to RR73/RRR msg
                break;

            // The Sequencer does not currently process certain message types.  We don't restart the Timer for unsupported msgs.
            case MSG_BLANK:
            case MSG_FREE:  // TODO:  we should try to handle this one someday
            case MSG_TELE:
            case MSG_UNKNOWN:
            default:
                DPRINTF("***** ERROR:  Unsupported received msgType=%d\n", msg->msgType);
                break;
        }  // switch
    }

}  // receivedMsgEvent()

/**
 * @brief Process received CQ message event
 *
 * If the operator has enabled automatic responses with the Tx button, the Sequencer ("RoboOp") automatically
 * responds to the first received CQ if we aren't already engaged in another activity (e.g. calling CQ ourself
 * or in an unfinished QSO).
 *
 * You may question why we need RoboOp.  Between the receipt of a CQ and the first opportunity to respond is
 * the FT8 "dwell" time of about 2.6 seconds (at most).  If we miss that timeslot opportunity, then we have to
 * sit out yet another timeslot (30 seconds so far) before the next chance to reply without doubling with the
 * remote station's CQ transmissions.  Thus, without RoboOp, we have only 2.6 secondes (at most) to
 * respond if we wish to avoid the 30 second penalty.
 */
void Sequencer::cqMsgEvent(Decode* msg) {
    // Has the operated enabled automatic replies (RoboOp) with the Tx button?
    if (!autoReplyToCQ) return;  // No... nothing to do here

    // Deal with previously logged duplicates
    String dupMsg;
    if (config.enableDuplicates) {
        dupMsg = String("RoboOp is responding to duplicate, ") + String(msg->field2);
        ui.applicationMsgs->setText(dupMsg.c_str());
    } else if (ContactLogFile::isKnownCallsign(msg->field2)) {
        dupMsg = String("RoboOp is ignoring duplicate, ") + String(msg->field2);
        ui.applicationMsgs->setText(dupMsg.c_str());
        return;  // RoboOp ignores stations already in the log
    }

    // Automatically respond to received CQ message if we are not already engaged in a QSO
    switch (state) {
        case IDLE:
            DTRACE();
            contact.begin(thisStation.getCallsign(), msg->field2, thisStation.getFrequency(), "FT8", thisStation.getRig(), ODD(msg->sequenceNumber));  // Start gathering contact info
            contact.setWorkedLocator(msg->field3);                                                                                                     // Record their locator if we recvd it
            setXmitParams(msg->field2, msg->snr);                                                                                                      // Inform gen_ft8 of remote station's info
            DPRINTF("Target_Call='%s', msg.field2='%s', msg.rsl=%d, Target_RSL=%d msg.sequenceNumber=%lu, contact.oddEven=%u\n", Target_Call, msg->field2, msg->snr, Target_RSL, msg->sequenceNumber, contact.oddEven);
            set_message(MSG_LOC);  // We get QSO underway by sending our locator
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);  // Display pending call in yellow
            startTimer();         // Start the Timer to terminate a run-on QSO
            state = LOC_PENDING;  // Await appropriate timeslot to transmit to Target_Call
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // Automatic responses are disabled if we are calling CQ or otherwise engaged in a QSO
        default:
            DTRACE();
            break;

    }  // switch
}  // cqMsgEvent()

/**
 * @brief Operator clicked the TUNE button
 */
void Sequencer::tuneButtonEvent() {
    // DTRACE();

    switch (state) {
        // Stop TUNING in-progress
        case TUNING:
            tune_Off_sequence();
            // ui.applicationMsgs->setText(" ");
            // resetButton(BUTTON_TU);
            ui.b2->reset();
            stopTimer();
            state = IDLE;
            // DTRACE();
            break;

        // Ignore TUNE button during ongoing transmission
        case XMIT_RSL:
        case XMIT_CQ:
        case XMIT_LOC:
        case XMIT_RRR:
        case XMIT_RRSL:
        case XMIT_73:
            break;

        // Start TUNING
        case IDLE:
        default:
            // DTRACE();
            //  Stop anything underway
            terminate_transmit_armed();
            xmit_flag = 0;  // Stop modulation in loop()

            // Transmit a dead, unmodulated carrier
            tune_On_sequence();
            state = TUNING;
            // ui.applicationMsgs->setText("TUNING");

            // Start a Timer to terminate TUNING if operator forgets
            // DTRACE();
            startTimer();
            break;
    }

}  // tuneButtonEvent();

/**
 *  Operator clicked the CQ button
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

            // Disable RoboOp's autoreply to CQ.  Our own CQ activity overides received CQ messages.
            setAutoReplyToCQ(false);

            // Prepare to call CQ
            set_message(MSG_CQ);  // Build our CQ message
            state = CQ_PENDING;   // Await the next timeslot
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);  // Display pending CQ message in yellow
            startTimer();  // Start the Timer to terminate run-on CQ transmissions
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // Abort CQ transmission (even if it's in progress)
        case CQ_PENDING:  // Pending timeslot to transmit CQ
        case XMIT_CQ:     // CQ transmission in progress
        case LISTEN_LOC:  // Listening for a response to our CQ message[s]
            DTRACE();
            state = IDLE;                   // Return to idle
            xmit_flag = 0;                  // Stop modulator
            terminate_transmit_armed();     // Disarm the transmitter
            clearOutboundMessageDisplay();  // Clears displayed outbound message as we're now idle
            stopTimer();                    // Stop the Timer
            // resetButton(BUTTON_CQ);         // Reset highlighted button
            ui.b0->reset();  // Reset highlighted button
            ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);
            break;

            // The CQ button is ignored during most states
        default:
            DTRACE();
            break;
    }
}

/**
 *  Clicked a decoded message to initiate a QSO
 *
 *  This event handler initiates a QSO by contacting a displayed, received
 *  message.
 *
 *  clickDecodedMessageEvent() is invoked only by display_selected_call() after initializing
 *  the Target_Call extern with the remote station's callsign.
 *
 *  We create a QSO contact here, perhaps prematurely (Target_Call may not reply),
 *  because we need an even/odd timeslot record of when Target_Call is transmitting.
 *  Also... this may be the only time when we'll have Target_Call's locator.
 *
 **/
void Sequencer::clickDecodedMessageEvent(unsigned msgIndex) {
    // Find the decoded message as we need some info from it
    Decode* msg = getDecodedMsg(msgIndex);
    clickDecodedMessageEvent(msg);
}  // clickDecodedMessageEvent()

void Sequencer::clickDecodedMessageEvent(Decode* msg) {
    // Assert Target_Call==msg->field2 as this stuff could become FUBAR
    DFPRINTF("sequenceNumber=%lu, Target_Call='%s', msg->field2='%s', msg->sequenceNumber=%u, state=%u\n", sequenceNumber, Target_Call, msg->field2, msg->sequenceNumber, state);

    // The required action depends upon which state the Synchronizer machine currently resides
    if (msg != NULL) {
        switch (state) {
            // Operator wants to send our grid locator to Target_Call
            case IDLE:        // We are currently idle
            case CQ_PENDING:  // Operator decided to contact a displayed station rather than call CQ
                DTRACE();
                contact.begin(thisStation.getCallsign(), msg->field2, thisStation.getFrequency(), "FT8", thisStation.getRig(), ODD(msg->sequenceNumber));  // Start gathering QSO info
                contact.setWorkedLocator(msg->field3);                                                                                                     // Record their locator if we have it
                setXmitParams(msg->field2, msg->snr);                                                                                                      // Inform gen_ft8 of remote station's info
                DPRINTF("Target_Call='%s', msg.field2='%s', msg.rsl=%d, Target_RSL=%d msg.sequenceNumber=%lu, contact.oddEven=%u\n", Target_Call, msg->field2, msg->snr, Target_RSL, msg->sequenceNumber, contact.oddEven);
                set_message(MSG_LOC);  // We initiate QSO by sending our locator
                // ui.applicationMsgs->setText(get_message(), A_YELLOW);  // Display pending call in yellow
                startTimer();  // Start the Timer to terminate a run-on QSO
                // resetButton(BUTTON_CQ);                        // No longer calling CQ
                ui.b0->reset();       // Reset highlighted button
                state = LOC_PENDING;  // Await appropriate timeslot to transmit to Target_Call
                ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);

                break;

                // Message clicks are ignored during most states
            default:
                DTRACE();
                break;
        }
    }

}  // clickDecodedMessageEvent()

/**
 * @brief Callback function notified if/when timeout Timer expires
 * @param thisTimer Pointer to the expiring Timer (not actually used)
 *
 * The timeout Timer limits the duration of run-on QSO and TUNING operations.
 * The Timer was created by begin(), started at the beginning of a QSO/TUNE
 * activity, and is normally stopped when that QSO/TUNE completes normally.
 * The duration was established by begin().
 */
void Sequencer::onTimerEvent(Timer* thisTimer) {
    // Find the one and only Sequencer from static method
    Sequencer& theSequencer = Sequencer::getSequencer();
    DFPRINTF("sequenceNumber=%lu, state=%u\n", theSequencer.sequenceNumber, theSequencer.state);

    // The Timer always halts the RoboOp from responding to CQs
    setAutoReplyToCQ(false);

    switch (theSequencer.state) {
        // We don't do anything if we were IDLE
        case IDLE:
            DTRACE();
            break;

        // Interrupt endless TUNING --- if it aint tuned by now, it aint gonna get tuned
        case TUNING:
            tune_Off_sequence();        // Stop the transmitted carrier
            theSequencer.state = IDLE;  // IDLE the state machine
            break;

        // Interrupt QSO listening for a specific response from the remote station
        case LISTEN_LOC:
        case LISTEN_RRR:
        case LISTEN_RRSL:
        case LISTEN_RSL:
        case LISTEN_73:
            theSequencer.highlightAbortedTransmission();
            theSequencer.endQSO();       // Misc activities to terminate QSO
            clearOutboundMessageText();  // Clear outbound message text chars
            theSequencer.state = IDLE;   // IDLE the state machine
            break;

        // Interrupt pending transmission awaiting an appropriate timeslot
        case CQ_PENDING:
        case LOC_PENDING:
        case RRR_PENDING:
        case RSL_PENDING:
        case RRSL_PENDING:
        case M73_PENDING:
            theSequencer.highlightAbortedTransmission();
            theSequencer.endQSO();
            clearOutboundMessageText();  // Clear outbound message text chars
            theSequencer.state = IDLE;   // IDLE the state machine
            break;

        // Interrupt active transmission (loop() is actively modulating symbols)
        case XMIT_CQ:
        case XMIT_LOC:
        case XMIT_RRR:
        case XMIT_RRSL:
        case XMIT_RSL:
        case XMIT_73:
            theSequencer.highlightAbortedTransmission();
            theSequencer.endQSO();
            clearOutboundMessageText();  // Clear outbound message text chars
            theSequencer.state = IDLE;   // IDLE the state machine
            break;

        // Interrupt a QSO gone bad (e.g. QRN, QRM, QSB, QRT, whatever) --- it's over
        default:
            break;
    }

    // Always reset a few more things
    theSequencer.stopTimer();  // Cancel the QSO Timer

    // Stop modulation, disarm the transmitter, clear the outbound message, and turn the receiver on
    xmit_flag = 0;                  // Stop modulation
    terminate_transmit_armed();     // Dis-arm the transmitter
    clearOutboundMessageDisplay();  // Clear displayed outbound message, if any
    receive_sequence();             // Only need to do this if in-progress transmission aborted

    // Reset highlighted buttons, if any
    // resetButton(BUTTON_TU);
    ui.b0->reset();  // Reset highlighted button
    ui.b2->reset();  // Reset highlighted button
                     // resetButton(BUTTON_CQ);

}  // onTimerEvent()

/**
 * @brief Operator clicked the ABORT button to halt transmissions
 *
 * For now, we handle this the same as when the QSO timeout Timer expires
 *
 */
void Sequencer::abortButtonEvent() {
    DTRACE();
    setAutoReplyToCQ(false);  // Reset the RoboOp
    onTimerEvent(NULL);       // TODO:  refactor so we don't need to pass a NULL Timer pointer
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);
}  // abortButtonEvent()

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
        // Remote station sent RRSL.  We'll send them an RRR to wrap things up.
        case LISTEN_RRSL:
            DTRACE();
            contact.setMyRSL(msg->field3);         // Record our signal report in QSO
            contact.setWorkedRSL(msg->snr);        // Record their RSL at the same time as ours
            setXmitParams(msg->field2, msg->snr);  // Inform gen_ft8 of remote station's info
            set_message(MSG_RRR);                  // Prepare an RRR message
            state = RRR_PENDING;                   // We must await an appropriate even/odd timeslot
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // Remote station sent our RSL and we should respond with their RRSL
        case LISTEN_RSL:
            DTRACE();
            contact.setMyRSL(msg->field3);         // Record our signal report in QSO structure
            contact.setWorkedRSL(msg->snr);        // Record their RSL at the same time as ours
            setXmitParams(msg->field2, msg->snr);  // Inform gen_ft8 of remote station's info
            set_message(MSG_RRSL);                 // Prepare to send their signal report to remote station
            state = RRSL_PENDING;                  // Await an appropriate even/odd timeslot
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // We were expecting a LOC but received a signal report.  We likely were calling CQ
        // and listening for a response which came with an RSL.  Let's try to cobble-up a
        // QSO from what we've heard.
        case LISTEN_LOC:
            DTRACE();
            contact.begin(thisStation.getCallsign(), msg->field2, thisStation.getFrequency(), "FT8", thisStation.getRig(), ODD(msg->sequenceNumber));  // Begin a QSO with their station
            contact.setMyRSL(msg->field3);                                                                                                             // Record our RSL from remote station
            contact.setWorkedRSL(msg->snr);                                                                                                            // Record their RSL at the same time as ours
            setXmitParams(msg->field2, msg->snr);                                                                                                      // Inform gen_ft8 of remote station's info
            DPRINTF("Target_Call='%s', msg.field2='%s', msg.rsl=%d, Target_RSL=%d msg.sequenceNumber=%lu, qso.oddEven=%u\n", Target_Call, msg->field2, msg->snr, Target_RSL, msg->sequenceNumber, contact.oddEven);
            set_message(MSG_RRSL);  // Roger our RSL and send their RSL to remote station
            state = RRSL_PENDING;   // Transmit their RSL in next appropriate timeslot
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // Ignore non-sense
        default:
            DPRINTF("***** NOTE:  Ignoring received msgType=%d from %s because state=%d\n", msg->msgType, msg->field2, state);
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
            endQSO();
            state = IDLE;  // It's definitely over now
            break;

        // We were trying to get a CQ transmission underway and can ignore this 73 which
        // presumably is left-over from a previous QSO.
        case CQ_PENDING:
        case XMIT_CQ:
        case LISTEN_LOC:
            DTRACE();
            break;

        // We were somewhere in midst of a QSO when the remote station 73'd.  QSO maybe incomplete.
        // TODO:  log???
        default:
            DTRACE();
            endQSO();
            state = IDLE;  // Return to IDLE
            break;
    }

    // This QSO is finished and we no longer need the Timer
    stopTimer();

}  // eotEvent()

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
            // ui.applicationMsgs->setText(get_message(), A_YELLOW);
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            // TODO:  qso.end()???
            break;

        // We were trying to get a CQ transmission underway and can ignore the EOT which
        // presumably is left-over from a previous QSO.
        case CQ_PENDING:
        case XMIT_CQ:
        case LISTEN_LOC:
            DTRACE();
            break;

        // We were somewhere in a QSO when the remote station 73'd.  QSO maybe incomplete.
        default:
            DTRACE();
            endQSO();
            state = IDLE;  // Return to IDLE
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
        case CQ_PENDING:  // We were going to [re]transmit CQ but received this msg first
        case LISTEN_LOC:  // We were listening for a response to our CQ and received this msg
            DTRACE();
            contact.begin(thisStation.getCallsign(), msg->field2, thisStation.getFrequency(), "FT8", thisStation.getRig(), ODD(sequenceNumber));  // Start gathering QSO info
            contact.setWorkedLocator(msg->field3);                                                                                                // Record responder's locator
            setXmitParams(msg->field2, msg->snr);                                                                                                 // Inform gen_ft8 of remote station's info
            DPRINTF("Target_Call='%s', msg.field2='%s', msg.rsl=%d, Target_RSL=%d msg.sequenceNumber=%lu, qso.oddEven=%u\n", Target_Call, msg->field2, msg->snr, Target_RSL, msg->sequenceNumber, contact.oddEven);
            set_message(MSG_RSL);  // Prepare to transmit RSL to responder
            state = RSL_PENDING;   // Must await an appropriate timeslot when responder is listening
            ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);
            break;

        // Something is wrong and we don't know why we apparently received their locator.  Maybe we should reset everything???
        default:
            DPRINTF("***** ERROR:  Received unexpected msgType=%d in state %d\n", msg->msgType, state);
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
    // A received msg is "for us" if our callsign or CQ appears as the destination station's callsign
    bool myCall = strncmp(msg->field1, thisStation.getCallsign(), sizeof(msg->field1)) == 0;  // Sent directly to us?
    bool cq = strcmp(msg->field1, "CQ") == 0;
    bool msgIsForUs = cq || myCall;
    return msgIsForUs;
}

/**
 *  Action routine to begin modulation in the requested odd/even timeslot
 *
 *  @param oddEven begin modulation in 1==odd, 0==even-numbered timeslot
 *  @param newState new state value if transmitter is armed
 *
 *  We don't actually modulate anything here, all we do is setup the global
 *  flags so loop() will begin transmitting FSK modulated symbols.
 *
 *  + You must invoke set_message() to prepare the outbound message prior
 *  to invoking pendXmit
 *
 *  + Upon entry, sequenceNumber identifies the *current* timeslot,
 *  not the next timeslot.  The oddEven parameter specifies whether the
 *  transmission should start in an odd or even-numbered timeslot.  To
 *  begin modulation in an odd-numbered timeslot, we must setup in
 *  an even-numbered timeslot.
 *
 *  We do nothing if the next timeslot is not appropriate.
 *
 **/
void Sequencer::pendXmit(unsigned oddEven, SequencerStateType newState) {
    oddEven &= 0x01;  // Force binary value:  1==odd, 0==even-numbered timeslot

    DFPRINTF("oddEven=%u, sequenceNumber=%lu newState=%u\n", oddEven, sequenceNumber, newState);

    // Arm the transmitter in the current timeslot if the required oddEven matches
    // the current sequenceNumber's oddEven to avoid doubling with the remote station.
    if (oddEven == ODD(sequenceNumber)) {
        Transmit_Armned = 1;                                // Yes, transmit in the next slot
        setup_to_transmit_on_next_DSP_Flag();               // loop() begins modulation in next timeslot
        state = newState;                                   // Advance state machine to new state after arming the transmitter
        ui.setXmitRecvIndicator(INDICATOR_ICON_TRANSMIT);   // Transmission will begin in loop()
        String thisTransmittedMsg = String(get_message());  // The pending outbound message text
        if (thisTransmittedMsg == lastTransmittedMsg) {
            ui.stationMsgs->setItemColors(lastStationMsgsItem, A_YELLOW, A_BLACK);  // Recolor previous (retransmitted) msg
        } else {
            lastStationMsgsItem = ui.stationMsgs->addStationMessageItem(ui.stationMsgs, thisTransmittedMsg);  // New transmitted msg
        }
        lastTransmittedMsg = thisTransmittedMsg;  // Remember for next time we add an item
        //DPRINTF("thisTransmittedMsg='%s'\n", thisTransmittedMsg.c_str());
    } else {
        ui.setXmitRecvIndicator(INDICATOR_ICON_PENDING);  // Transmission pending appropriate time slot
    }

}  // pendXmit()

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
}  // getSequenceNumber()

/**
 * @brief Get state for debugging Sequencer problems
 * @return state
 */
SequencerStateType Sequencer::getState() {
    return state;
}  // getState()

/**
 * @brief Start the QSO timeout timer
 *
 * The Timer was created and its interval established by begin().
 *
 * If/when the Timer expires, it notifies onTimerEvent() callback function.
 *
 */
void Sequencer::startTimer() {
    timeoutTimer->start();
}

/**
 * @brief Stop the running timeout timer
 */
void Sequencer::stopTimer() {
    timeoutTimer->stop();
}

/**
 * @brief Terminate a QSO
 *
 * Helper routine manages a variety of details involved in ending a QSO:
 *
 *  + Log a contact if possible
 *  + Clear an outstanding FT8 message, if any
 *  + Clear the info display message, if any
 *  + Reset highlighted buttons
 *  + Cancel the timeout Timer
 *  + Switch from transmit to receive
 *
 */
void Sequencer::endQSO() {
    DTRACE();

    // Add info to log
    contact.setRig(thisStation.getRig());
    contact.setPwr(0.250);
    contact.setMyLocator(thisStation.getLocator());
    contact.setMyName(thisStation.getMyName());

    // Log the contact if we collected sufficient data about the remote station
    if (contact.isActive() && contact.isValid()) {
        DTRACE();
        contactLog->logContact(&contact);
        String str = String("Logged ") + String(contact.getWorkedCall());
        ui.applicationMsgs->setText(str);
    }

    // Cancel the QSO timeout Timer
    stopTimer();

    // Disarm the transmitter, clear the outbound message, and turn the receiver on
    xmit_flag = 0;                  // Stop modulation
    terminate_transmit_armed();     // Dis-arm the transmitter
    clearOutboundMessageDisplay();  // Clear displayed outbound message, if any
    receive_sequence();             // Only need to do this if in-progress transmission aborted

    // Clean-up GUI leftovers
    ui.b0->reset();  // Reset highlighted button
    ui.b2->reset();  // Reset highlighted button

    // Reset the RoboOp auto reply
    setAutoReplyToCQ(false);

    // We are finished with this contact whether we had enough data to log it or not
    contact.reset();
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);  // We are receiving again
}

/**
 * @brief Setter for autoReplyToCQ
 * @param x true enables auto reply to CQ
 *
 * The RoboOp sequencer automatically replies to a non-directed CQ message if
 * autoReplyToCQ==true.  We also reset the Tx button when autoReply clears.
 *
 */
void setAutoReplyToCQ(bool x) {
    autoReplyToCQ = x;
    DPRINTF("autoReplyToCQ=%d\n", autoReplyToCQ);
    if (!autoReplyToCQ) {
        ui.b3->reset();  // Reset highlighted button
    }
}

/**
 * @brief getter for autoReplyToCQ
 * @return true or false
 */
bool getAutoReplyToCQ() {
    return autoReplyToCQ;
}

/**
 * @brief Highlight the lastTransmittedMsg if it timed-out
 */
void Sequencer::highlightAbortedTransmission() {
    String thisTransmittedMsg = String(get_message());  // The pending outbound message text
    if (thisTransmittedMsg == lastTransmittedMsg) {
        ui.stationMsgs->setItemColors(lastStationMsgsItem, A_DARK_GREY, A_BLACK);  // Recolor previous (retransmitted) msg
    }
}  // highlightAbortedTransmission()