/**
 * @brief Interface between legacy Pocket FT8 code and the current lib_ft8
 *
 * @note The github ft8_lib code underwent substantial changes independent of the
 * Pocket FT8 development.  Here we implement the legacy ft8_lib interface to the
 * October 2025 version of ft8_lib (to allow use of the new lib code without
 * massive changes to the Pocket FT8 code), and minimal changes to ft8_lib.
 */

#include <Arduino.h>
#include <map>
#include "ft8LibIfce.h"
#include "message.h"
#include "DEBUG.h"
#include "text.h"

std::map<uint32_t, String> nonStandardCallsignTable;  // Surprise:  Implemented as an ordered map!

/**
 * @brief Record an entry in the nonStandardCallsignTable map for key
 * @param callsign The callsign to be associated with key
 * @param key22 Apparently caller always supplies a 22-bit key
 *
 * @note We don't actually use a hash table as we've already loaded std::map
 *
 * @note Recording an entry with a previously used key overwrites the existing entry
 */
static void save_hash(const char* callsign, uint32_t key22) {
    uint32_t key10 = (key22 >> 12) & 0x3ff;  // Entries are apparently recorded using a 10-bit key
    nonStandardCallsignTable[key10] = callsign;
    DPRINTF("save_hash('%s',key22=%d) used key10=%d, size()=%d\n", callsign, key22, key10, nonStandardCallsignTable.size());

}  // add()

/**
 * @brief Lookup an entry in the nonStandardCallsignTable for key
 * @param hash_type #bits in the supplied key
 * @param key The supplied key
 * @param c11 Buffer to receive the callsign
 * @return true==success
 */
static bool lookup_hash(ftx_callsign_hash_type_t hash_type, uint32_t key, char* c11) {
    // DPRINTF("lookup_hash(hash_type=%d, key=%d)\n", hash_type, key);
    //  Callsigns apparently are always recorded and retrieved using a 10-bit key
    uint32_t key10;  // A 10-bit version of key
    switch (hash_type) {
        case FTX_CALLSIGN_HASH_10_BITS:  // Caller supplied a 10-bit key
            key10 = key;                 // Use supplied key as-is
            break;
        case FTX_CALLSIGN_HASH_12_BITS:  // Caller supplied a 12-bit key
            key10 = key >> 2;            // Make a 10-bit key
            break;
        case FTX_CALLSIGN_HASH_22_BITS:  // Caller supplied a 22-bit key
            key10 = key >> 12;           // Make a 10-bit key
            break;
        default:         // Caller supplied nonsense
            key10 = -1;  // Then use nonsense key
            break;
    }
    key10 = key10 & 0x3ff;
    // DPRINTF("Calculated key10=%d\n", key10);
    String s = nonStandardCallsignTable[key10];  // Always lookup callsign using a 10-bit key
    // DPRINTF("lookup_hash(key10=%d) retrieved '%s'\n", key10, s.c_str());
    if (s.length() > 0) {
        strlcpy(c11, s.c_str(), 12);
        return true;
    } else {
        return false;
    }
}  // lookup()

/**
 * @brief ft8_lib-defined struct of pointers to our callsign hash map
 *
 * @note This struct provides ft8_lib with access to the Pocket FT8 callsign hash map
 */
static ftx_callsign_hash_interface_t hashingIfce = {lookup_hash, save_hash};

/**
 * @brief Unpack demodulated message bits into readable text
 * @param a77 The 77-bit demodulated message
 * @param field1 char[14] in which to place FT8 message field1
 * @param field2 char[14] in which to place FT8 message field2
 * @param field3 char[7] in which to place FT8 message field3
 * @param msgType Returns legacy ft8_lib MsgType
 * @return 0==success
 *
 */
int unpack77_fields(const uint8_t* a77, char* field1, char* field2, char* field3, MsgType* msgType) {
    ftx_message_t demodMsg;
    char resultTxt[35];                  // Message text
    ftx_message_offsets_t resultFields;  // The offsets of the fields in msgText

    DTRACE();

    // Initialize a few things
    field1[0] = field2[0] = field3[0] = 0;  // The fields are all empty strings
    *msgType = MSG_UNKNOWN;                 // Assume failure for now

    // Build ft8_lib's demodulated message structure
    memcpy(demodMsg.payload, a77, sizeof(demodMsg.payload));
    demodMsg.hash = 0;  // We curently aren't using the check-for-duplicates hash of entire message

    // Unpack a demodulated message into a char[] string and info about the three ft8 fields
    const char delimitor[] = " ";
    ftx_message_rc_t rc = ftx_message_decode(&demodMsg, &hashingIfce, resultTxt, &resultFields);
    // DTRACE();

    // Copy free text message into field1
    if (resultFields.types[0] == FTX_FIELD_FREETEXT) {
        // DPRINTF("resultTxt='%s'\n", resultTxt);
        // DPRINTF("field1='%s'\n", field1);
        strlcpy(field1, resultTxt, FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE);
        // DPRINTF("field1='%s'\n", field1);
    }

    // Extract field1 from resultTxt
    else if (resultFields.types[0] != FTX_FIELD_NONE) {
        strlcpy(field1, strtok(resultTxt + resultFields.offsets[0], delimitor), FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE);
    }
    // DPRINTF("field1='%s'\n", field1);

    // Extract field2 from resultTxt
    if (resultFields.types[1] != FTX_FIELD_NONE) {
        strlcpy(field2, strtok(resultTxt + resultFields.offsets[1], delimitor), FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE);
    }
    // DPRINTF("field1='%s'\n", field1);

    // Extract field3 from resultTxt
    if (resultFields.types[2] != FTX_FIELD_NONE) {
        strlcpy(field3, strtok(resultTxt + resultFields.offsets[2], delimitor), FTX_REPORTS_BFRSIZE);
    }
    // DPRINTF("field1='%s'\n", field1);

    // Recover the legacy ft8_lib msgType from today's field types and content returned by ftx_message_decode().
    // Note:  Avoid confusion between the legacy MsgType and today's ftx_message_type_t --- they're related but different.
    // LIMITATION:  We don't distinguish between MSG_FREE and MSG_TELE.
    if ((resultFields.types[0] == FTX_FIELD_TOKEN) || (resultFields.types[0] == FTX_FIELD_TOKEN_WITH_ARG)) {
        *msgType = MSG_CQ;  // CQ or CQ xxxx message
    } else if ((resultFields.types[0] == FTX_FIELD_FREETEXT) && (strlen(field1) > 0)) {
        *msgType = MSG_FREE;  // Free text message
        // DPRINTF("field1='%s'\n", field1);
    } else if (resultFields.types[2] == FTX_FIELD_RST) {
        *msgType = MSG_RSL;  // Either RSL or RRSL (Sequencer doesn't care)
    } else if (resultFields.types[2] == FTX_FIELD_GRID) {
        *msgType = MSG_LOC;  // Locator
    } else if (resultFields.types[2] == FTX_FIELD_TOKEN) {
        if (strncmp(resultTxt + resultFields.offsets[2], "73", 6) == 0) *msgType = MSG_73;
        if (strncmp(resultTxt + resultFields.offsets[2], "RR73", 6) == 0) *msgType = MSG_RR73;
        if (strncmp(resultTxt + resultFields.offsets[2], "RRR", 6) == 0) *msgType = MSG_RRR;
    }

    DPRINTF("field1='%s' field2='%s' field3='%s' rc=%d msgType=%d\n", field1, field2, field3, rc, *msgType);
    // DPRINTF("types1=%d   types2=%d   types3=%d   msgType=%d\n", resultFields.types[0], resultFields.types[1], resultFields.types[2], *msgType);

    return rc;  // 0==success
}  // unpack77_fields()

/**
 * @brief Pack any supported FT8 text message into a 77-bit array
 * @param msg Message text
 * @param b77 The 77-bit array receives the packed message
 * @return 0==success
 *
 * @note While result includes a hash member, apparently for identifying duplicate messages,
 * it does not appear to be in-use anywhere.
 */
int pack77(const char* msg, uint8_t* b77) {
    ftx_message_t result;

    DPRINTF("pack77(msg='%s')\n", msg);

    // Pack msg[] into result.  We aren't using the callsign hash interface in Pocket FT8.
    result.hash = 0;                                                       // The duplicate message hash may not be in use???
    ftx_message_rc_t rc = ftx_message_encode(&result, &hashingIfce, msg);  // Pack text msg into result
    memcpy(b77, result.payload, FTX_PAYLOAD_LENGTH_BYTES);                 // Return resulting payload

    // DPRINTF("rc=%d\n", rc);

    return rc;
}  // pack77()

/**
 * @brief Trim angle brackets from a callsign string in-place
 * @param s Callsign string (may be NULL), too-short, or even empty
 *
 * @note The callsign string may or may not actually include angle brackets
 *
 * @note If angle brackets are present, the callsign string is modified in-place.
 *       The modified string will be shorter than the original.
 *
 * @note Malformed strings (too short, too long, missing brackets, NULL)
 *       remain unmodified.
 */
void trimBracketsFromCallsign(char* s) {
    // DPRINTF("trimCallsign('%s')\n", s);
    //   Check for malformed callsign strings we can ignore
    if (s == NULL) return;                             // No string to trim
    int len = strlen(s);                               //
    if (len < 3) return;                               // Too short to be a hashed callsign with brackets?
    if (len > 13) return;                              // Too long to be any valid FT8 callsign?
    if ((s[0] != '<') || (s[len - 1] != '>')) return;  // Perhaps the brackets are missing?

    // Trim angle brackets from the hashed callsign in s
    memmove(s, s + 1, len - 1);  // Remove the first bracket from s
    s[len - 2] = 0;              // Remove the second bracket from s
    // DPRINTF("trimCallsign()='%s'\n", s);

}  // trimCallsign