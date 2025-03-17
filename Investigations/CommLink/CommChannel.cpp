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
 *  CommChannel myLink(0);          //Build channel 0
 *  int err = myLink.connect(60);   //Establish connection with 60 second timeout
 *  int c = myLink.read();          //Read one byte
 *  myLink.disconnect();            //Disconnect from remote
 *
 */

/**
 * @brief Build an instance of the specified channel
 * @param channelNumber The specified channel number
 *
 * IMPLEMENTATION
 * All we do here is construct the CommChannel object and record its location in
 * allChannels[].  The actual connection, and resulting network traffic, occurs
 * in connect().
 *
 * LIMITATIONS
 * It would be nice to verify the requested channel isn't already in use
 * but, at least for now, that's not our problem (so don't foul it up).   Along
 * that line, perhaps we should implement a destructor someday as well.
 *
 */
CommChannel::CommChannel(unsigned channelNumber) {
    allChannels[channelNumber] = this;
}  // CommChannel(channelNumber)

/**
 * @brief Initiate a connection with this channel to remote endpoint
 * @return 0=success, else an error code as described below
 *
 * ERROR CODES
 *  ERR_OK      No error (connection established)
 *  ERR_BUSY    ARQ busy (try again)
 *  ERR_PENDING Connection request pending (keep waiting)
 *  ERR_DISCO   Remote disconnected (not good)
 *
 * NOTES
 *  + To successfully establish a connection, both the local and remote host, at both
 *  ends of the physical link (e.g. USB), must invoke connect().
 *  + The two endpoints likely invoke connect() at different times.  The
 *  timeoutSeconds determines how long this endpoint will wait for the other
 *  endpoint's connect() request.
 *  + The implementation assumes the ARQ layer reliably transmits the
 *  connection request --- it is not retried here in CommChannel.
 *
 */
int CommChannel::connect() {
    return -1;
}  // connect()

/**
 * @brief Disconnect from the remote endpoint
 */
void CommChannel::disconnect() {
    return;
}

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