/**
 * @brief Interface between legacy Pocket FT8 code and the current lib_ft8
 */

#include <Arduino.h>
#include "ft8LibIfce.h"
#include "message.h"
#include "DEBUG.h"

/**
 * @brief Unpack demodulated message bits into readable text
 * @param a77 The 77-bit demodulated message
 * @param field1
 * @param field2
 * @param field3
 * @param msgType
 * @return 0==success
 */
int unpack77_fields(const uint8_t* a77, char* field1, char* field2, char* field3, MsgType* msgType) {
    ftx_message_t demodMsg;
    char msgText[35];               // Message text
    ftx_message_offsets_t offsets;  // The offsets of the fields in msgText

    DTRACE();

    // Build the demodulated message structure
    memcpy(demodMsg.payload, a77, sizeof(demodMsg.payload));
    demodMsg.hash = 0;  // We curently aren't using the check-for-duplicates hash of entire message

    ftx_message_rc_t rc = ftx_message_decode(&demodMsg, NULL, msgText, field1, field2, field3, &offsets);
    DPRINTF("field1='%s' field2='%s' field3='%s' rc=%d\n", field1, field2, field3, rc);
    return rc;  // 0==success
}  // unpack77_fields()

/**
 * @brief Pack any supported FT8 text message into a 77-bit array
 * @param msg Message text
 * @param b77 The 77-bit array receives the packed message
 * @return 0==success
 */
int pack77(const char* msg, uint8_t* b77) {
    ftx_message_t result;

    DPRINTF("msg='%s'\n", msg);

    // Pack msg[] into result.  We aren't using the callsign hash interface in Pocket FT8.
    result.hash = 0;                                               // The duplicate message hash may not be in use???
    ftx_message_rc_t rc = ftx_message_encode(&result, NULL, msg);  // Pack text msg into result
    memcpy(b77, result.payload, FTX_PAYLOAD_LENGTH_BYTES);

    DPRINTF("rc=%d\n", rc);

    return rc;
} //pack77()
