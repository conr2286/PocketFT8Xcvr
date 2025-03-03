#include "CommChannel.h"

#include "CommLink.h"

/**
 * @brief Implements a connection-oriented byte-stream virtual circuit over a reliable frame service
 *
 * CommChannel is based upon Stream which in turn is based upon the Arduino Print class (these
 * must be mocked for native builds).
 *
 * RESPONSIBILITIES
 * + Establish a connection (i.e. virtual circuit) with a remote endpoint
 * + Transmit/receive a stream of bytes through the connection
 * + "Packetizing" the byte stream into data link frames
 * + Graceful disconnection from the remote endpoint (ensures it's not over till it's over)
 * + Re-synchronization of the protocol stack (e.g. recovery from crash/reboot of MCU)
 *
 * LIMITATIONS
 * There is a strong assumption of no host addressing in that the connection exists over a
 * reliable point-to-point data link between exactly two computing systems.
 *
 * DESIGN NOTES
 * The CommLink library implementation is mostly, but not completely, a layered architecture.
 * For example, most of the Channel and Frame code is implemented in separate classes with
 * reasonable division of responsibilities, but the frame header has some cross-coupling
 * (in the endless pursuit of channel efficiency which is otherwise pretty much toast herein).
 *
 * USAGE
 *
 *
 */

/**
 * @brief Write one byte to a CommStream
 * @param b Byte to write
 * @return #bytes actually written
 */
size_t CommChannel::write(uint8_t b) {
    return 0;
}

/**
 * @brief Return count of bytes available to read from this channel
 * @return count
 */
int CommChannel::available() {
    return 0;
}

/**
 * @brief Returns one byte received on this channel
 * @return Received byte or -1 if none available
 */
int CommChannel::read() {
    return -1;
}

/**
 * @brief Peek at (without removing from channel buffer) the next received byte to read
 * @return Received byte or -1 if none available
 */
int CommChannel::peek() {
    return -1;
}