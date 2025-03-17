/**
 * @brief Stop-and-Wait with Automatic Repeat reQuest (ARQ) Data Link Layer for the CommLink Library
 *
 * DESIGN NOTES
 * The CommLink network consists of three layers:
 *  + CommChannel:  Implements a byte stream transport through a channel (virtual circuit) connection
 *  + CommARQ:  Implements a reliable data link using stop-and-wait ARQ
 *  + CommPtP:  Physical point-to-point (e.g. USB Serial) connection
 * Note the absence of a network layer and anything related to addressing as CommLink is designed to
 * provide connectivity between an MCU (e.g. Arduino, especially a Teensy) and its host computer via
 * a point-to-point connection.
 *
 * CommLink emphasizes simplicity over efficiency, hence the use of a stop-and-wait protocol.  It's
 * intended for transferring configuration data, log files, etc, not for video or other high
 * performance applications.
 *
 * CommLink will build in the Arduino environment or for a Unix-like (native) environment.  Unix
 * configurations mock-up some Arduino classes (e.g. Stream, Serial...), especially for testing.
 * Nearly if not all CommLink methods are non-blocking and use polling to resume work-in-progress
 * that otherwise would have blocked.
 *
 * Flow Control:  CommARQ implements a stop-and-wait protocol to regulate the flow of
 * frames through the physical link.  Received frame buffers are added to a channel's inbound
 * buffer queue.  The channel, after extracting its data, returns an empty buffer to CommARQ
 * through releaseBfr() which arranges to transmit an appropriate ACK, allowing the remote
 * endpoint to transmit its next frame to us. CommARQ only transmits an ACK when it has an
 * available buffer to receive an incoming frame.
 *
 */

#include "CommARQ.h"

#include "CommBfr.h"
#include "CommChannel.h"
#include "CommLink.h"
#include "CommPhy.h"
#include "MessageTypes.h"
#include "radix64.h"

/**
 * @brief Build a CommARQ instance
 * @param phyLink Pointer to the CommPtP instance to use for the physical link
 */
CommARQ::CommARQ(CommPhy* commPhy) {
    this->commPhy = commPhy;  // Bind this link-layer instance to a physical-layer instance
    commPhy->begin();         // Start the physical layer
}  // CommARQ(commPhy)

/**
 * @brief Poll this data link for work-in-progress
 *
 * @note The implementation of poll() assumes it will be called repeatedly from loop() to
 * service the ARQ (data link) layer.  Responsibilities include:
 *  + read received byte(s) from physical layer
 *  + decode the received frame
 *  + add the frame buffer to channel's queue
 *  + obtain an empty buffer from TODO WHICH channel's free pool
 */
void CommARQ::poll() {
    char b;
    // Copy chars from phys link into phyRecvBfr[] until we detect End-of-Message (EOM)
    while (commPhy->available() && (b = commPhy->getch()) != EOM) {
        processReceivedChar(b);  // Append received char to frame buffer
    }

    // We are finished in here if we haven't received the entire frame yet
    if (b != EOM) return;

    // Decode the physical buffer into a free frame buffer and check for corner cases

    // Decode the Radix64 data into a free buffer (we're supposed to have one)
    CommBfr* recvFrame = recvFreePool.remove();                             // Get a free buffer
    if (recvFrame == NULL) return;                                          // Something went terribly wrong
    size_t size;                                                            // #bytes in unpacked frame
    Radix64::decodeRadix64((char*)&(recvFrame->frame), phyRecvBfr, &size);  // Decode

    // Check calculated CRC with received CRC, discarding invalid frames
    if (!verifyCRC(recvFrame)) {
        recvFreePool.add(recvFrame);  // Return discarded frame to our free pool
        return;                       // Exit without further analysis of received frame
    }

    // We have received a valid frame.  Check for a datalink reset (remote may have rebooted).
    if (recvFrame->frame.arqMsgHdr == ARQ_RST) {
        recvFreePool.add(recvFrame);  // Return received frame to free pool
        resetCommLink();              // Reset everything (we are starting over)
        return;                       // Nothing else to do
    }

    // With the corner cases out of the way, we can now process the received frame

    // Find the destination CommChannel object, discarding received frames for inactive channel numbers
    CommChannel* destChannel = CommChannel::getCommChannel(recvFrame->frame.chnNumber);  // Map channel number to pointer
    if (destChannel == NULL) {
        sendChannelReset(recvFrame->frame.chnNumber);  // Inform remote channel endpoint we are inactive
        recvFreePool.add(recvFrame);                   // Free our frame buffer
        return;                                        // Finished with this frame
    }

    switch (recvFrame->frame.arqMsgHdr) {
        // Push DAT0 frame into the specified CommChannel
        case ARQ_DAT0:
            recvDAT0(destChannel, recvFrame);
            break;

        // Push DAT1 frame into the specified CommChannel
        case ARQ_DAT1:
            recvDAT1(destChannel, recvFrame);
            break;

        // Notify the transmitter upon receipt of ACK0.
        case ARQ_ACK0:
            recvACK0();
            break;

        // Notify transmitter upon receipt of ACK1
        case ARQ_ACK1:
            recvACK1();
            break;

        // Something is terribly wrong
        default:
            break;
    }

    // Return received frame's buffer to the free pool
    recvFreePool.add(recvFrame);

}  // poll()

/**
 * @brief Process received DAT0 frame
 * @param chn Destination channel for frame data
 * @param bfr Buffer containing the received frame
 */
void CommARQ::recvDAT0(CommChannel* chn, CommBfr* bfr) {
    switch (recvState) {
        // Is our receiver awaiting a DAT0 frame?
        case RECV_STATE_WDAT0:
            chn->addFrame(bfr);           // Add the received frame to the destination channel's FIFO
            recvState = RECV_STATE_BFR0;  // Now we await channel to consume the frame
            break;

        // Remote is retransmitting DAT0 before we've sent them our ACK0.  Ignore duplicate DAT0.
        case RECV_STATE_BFR0:
            break;

        // We are awaiting DAT1 but received a DAT0.  The remote is most likely retransmitting DAT0
        // because it didn't receive our ACK0.  Let's retransmit the ACK0.
        case RECV_STATE_WDAT1:
            xmitACK0();
            break;

        // We are waiting to send ACK1 but received a DAT0.  Perhaps the remote rebooted???
        case RECV_STATE_BFR1:  // Waiting to send ACK1
        default:               // Something is horribly wrong
            resetCommLink();   // Reset everything to recover
            break;
    }
}  // recvDAT0()

/**
 * @brief Process received DAT1 frame
 * @param chn Destination channel number
 * @param bfr Buffer containing the received frame
 */
void CommARQ::recvDAT1(CommChannel* chn, CommBfr* bfr) {
    switch (recvState) {
        // Is our receiver awaiting a DAT1 frame?
        case RECV_STATE_WDAT1:
            chn->addFrame(bfr);           // Add the received frame to the destination channel's FIFO
            recvState = RECV_STATE_BFR1;  // Now we await channel to consume the frame
            break;

        // Remote is retransmitting DAT1 before we've sent them our ACK1.  Ignore duplicate DAT1.
        case RECV_STATE_BFR1:
            break;

        // We are awaiting DAT0 but received a DAT1.  The remote is most likely retransmitting DAT1
        // because it didn't receive our ACK1.  Let's retransmit the ACK1.
        case RECV_STATE_WDAT0:
            xmitACK1();
            break;

        // We are waiting to send ACK0 but received a DAT1.  Perhaps the remote rebooted???
        case RECV_STATE_BFR0:  // Waiting to send ACK0
        default:               // Something horribly wrong
            resetCommLink();   // Reset everything to recover
            break;
    }
}  // recvDAT1()

/**
 * @brief We have received an ACK0 for a DAT0 frame we presumably previously transmitted
 */
void CommARQ::recvACK0() {
}

/**
 * @brief We have received an ACK1 for a DAT1 frame we presumably previously transmitted
 */
void CommARQ::recvACK1() {
}

/**
 * @brief Send the specified buffer to the remote host's ARQ layer
 * @param bfr Specified frame buffer pointer
 *
 * DESIGN NOTES
 * This is the stop-and-wait ARQ transmitter.  The CommChannel layer assembles a complete
 * frame buffer and invokes sendBfr() to transmit it.  The sendBfr() code and its
 * helpers are resonsible for transmitting the frame through the physical link (e.g. Serial),
 * implementing the ARQ timeout, and retransmitting unacknowledged frames, with the
 * transmitter's ARQ state machine.
 */
void CommARQ::sendBfr(CommBfr* bfr) {
    // Validate parameters
    if (bfr == NULL) return;  // Something is terribly wrong

    // Add the buffer to the transmitter's outbound FIFO
    xmitQueue.add(bfr);  // Queued for eventual transmission

    // Perhaps we can immediately send this buffer
    switch (xmitState) {
        // Is the transmitter waiting to send a DAT0 frame?
        case XMIT_STATE_RDY0:
            xmitDAT0();  // Yes, go send it
            break;

        // Is the transmitter waiting to send a DAT1 frame?
        case XMIT_STATE_RDY1:
            xmitDAT1();  // Yes, go send it
            break;

        // Bfr must await its turn if transmitter is otherwise busy
        case XMIT_STATE_WACK0:
        case XMIT_STATE_WACK1:
        default:
            break;
    }
}

/**
 * @brief Transmit DAT0 frame to the remote ARQ
 *
 * The head of the xmitQueue is the outbound frame.  Our responsibilities:
 *  + Encode the frame into the phyXmitBfr[]
 *  + Transmit phyXmitBfr[] over the physical link
 *  + Start the ARQ timer
 *  + Leave the frame at head of queue until receiver hears its ACK0
 *
 * NOTES
 *  If the phyXmitBfr[] is sufficient in size, then the physical link (e.g. Serial) will
 *  likely block (e.g. loop waiting for bytes to clock out).  Under "normal" conditions,
 *  this likely isn't a significant problem.  But if, for example, the Serial link is
 *  disconnected then the physical link may timeout and fail to send the byte.  Since
 *  the physical link isn't working anyway, we ignore this error and allow the ARQ to
 *  retransmit the frame later --- we've done what we can until the link is fixed.
 */
void CommARQ::xmitDAT0() {
    // Encode the head of the transmit queue into the phyXmitBfr
    CommBfr* xmitFrame = xmitQueue.peek();  // Peek at (without removing) the head of the queue
    if (xmitFrame == NULL) return;          // Something is terribly wrong :(
    // Radix64::decodeRadix64((char*)&(recvFrame->frame), phyRecvBfr, &size);  // Decode
    encodeRadix64(phyXmitBfr, (char*)&(xmitFrame->frame), input_length);
}

/**
 * @brief Transmit DAT1 frame to the remote ARQ
 *
 *  The head of the xmitQueue is the outbound frame.  Our responsibilities:
 *  + Encode the frame into the phyXmitBfr[]
 *  + Transmit phyXmitBfr[] over the physical link
 *  + Start the ARQ timer
 *  + Leave the frame at head of queue until receiver hears its ACK1
 *
 *  * NOTES
 *  If the phyXmitBfr[] is sufficient in size, then the physical link (e.g. Serial) will
 *  likely block (e.g. loop waiting for bytes to clock out).  Under "normal" conditions,
 *  this likely isn't a significant problem.  But if, for example, the Serial link is
 *  disconnected then the physical link may timeout and fail to send the byte.  Since
 *  the physical link isn't working anyway, we ignore this error and allow the ARQ to
 *  retransmit the frame later --- we've done what we can until the link is fixed.
 */
void CommARQ::xmitDAT1() {
}