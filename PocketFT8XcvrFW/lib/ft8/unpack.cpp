
#include <string.h>

#include "msgTypes.h"
#include "DEBUG.h"
#include "unpack.h"
#include "text.h"
#include "FT8CallsignHashTable.h"

// extern _Bool true;
// extern _Bool false;

// const uint32_t NBASE = 37L*36L*10L*27L*27L*27L;
const uint32_t MAX22 = 4194304L;
const uint32_t NTOKENS = 2063592L;
const uint16_t MAXGRID4 = 32400L;

// Implement the callsign hash table
FT8CallsignHashTable callsignHashTable;

/**
 * SYNOPSIS
 *  unpack --- Functions for unpacking FT8 message text from the 77-bit FT8 payload
 *
 * DESCRIPTION
 *  The main entry is unpack77_fields(); the other functions are helpers.
 *
 *  FT8 messages are densely packed to improve channel utilization as described in,
 *  Franke, Somerville and Taylor.  "The FT4 and FT8 Communication Protocols."
 *  QEX.  July/August 2020.
 *
 *  Unpack has significant insight into the content and meaning of the various
 *  message fields, especially the so-called field3.
 *
 * MIT LICENSE
 * Copyright (c) 2018 Kārlis Goba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **/

/**
 * Unpack callsign bits
 *
 * @param n28 28-bit integer, e.g. n28a or n28b, containing the call sign bits from a packed message.
 * @param ip  if true then check for /R or /P
 * @param i3  1==/R, 2==/P
 * @param result[] plaintext callsign
 *
 **/
int unpack28(uint32_t n28, uint8_t ip, uint8_t i3, char *result) {
    // Check for special tokens DE, QRZ, CQ, CQ_nnn, CQ_aaaa
    if (n28 < NTOKENS) {
        if (n28 <= 2) {
            if (n28 == 0) strcpy(result, "DE");
            if (n28 == 1) strcpy(result, "QRZ");
            if (n28 == 2) strcpy(result, "CQ");
            return 0;  // Success
        }
        if (n28 <= 1002) {
            // CQ_nnn with 3 digits
            strcpy(result, "CQ ");
            int_to_dd(result + 3, n28 - 3, 3, true);
            return 0;  // Success
        }
        if (n28 <= 532443L) {
            // CQ_aaaa with 4 alphanumeric symbols
            uint32_t n = n28 - 1003;
            char aaaa[5];

            aaaa[4] = '\0';
            for (int i = 3; /* */; --i) {
                aaaa[i] = charn(n % 27, 4);
                if (i == 0) break;
                n /= 27;
            }

            strcpy(result, "CQ ");
            strcat(result, trim_front(aaaa));
            return 0;  // Success
        }
        // ? TODO: unspecified in the WSJT-X code
        return -1;
    }

    n28 = n28 - NTOKENS;
    if (n28 < MAX22) {
        // This n28 is actually a 22-bit hash of a result

        String s = callsignHashTable.lookup(n28);  // Map the 22-bit hash key to a callsign string
        if (s.length() != 0) {
            s = String("<") + s + String(">");  // Enclose callsign in angle brackets
        } else {
            s = "<...>";  // Deal with an unknown hash key
        }
        strlcpy(result, s.c_str(), sizeof(result));  // Legacy C code expects a char[]
        DPRINTF("lookup(%u)='%s'\n", n28, result);
        // result[0] = '<';
        // int_to_dd(result + 1, n28, 7, true);
        // result[8] = '>';
        // result[9] = '\0';
        return 0;
    }

    // Standard callsign
    uint32_t n = n28 - MAX22;

    char callsign[7];
    callsign[6] = '\0';
    callsign[5] = charn(n % 27, 4);
    n /= 27;
    callsign[4] = charn(n % 27, 4);
    n /= 27;
    callsign[3] = charn(n % 27, 4);
    n /= 27;
    callsign[2] = charn(n % 10, 3);
    n /= 10;
    callsign[1] = charn(n % 36, 2);
    n /= 36;
    callsign[0] = charn(n % 37, 1);

    // Skip trailing and leading whitespace in case of a short callsign
    strcpy(result, trim(callsign));
    if (strlen(result) == 0) return -1;

    // Check if we should append /R or /P suffix
    if (ip) {
        if (i3 == 1) {
            strcat(result, "/R");
        } else if (i3 == 2) {
            strcat(result, "/P");
        }
    }

    DPRINTF("legacy save_callsign '%s'\n", result);  // Why would ft8_lib want to save a standard callsign in hash table???

    return 0;  // Success
}

/**
 * Unpack the FT8 "Standard Message"
 *
 *  @param a77 Packed, received, 77-bit, FT8 message
 *  @param i3  Message type
 *  @param field1 Buffer to receive field1 text, usually the destination station's callsign
 *  @param field2 Buffer to receive field2 text, usually the source station's callsign
 *  @param field3 Buffer to receive field3 text
 *  @param *msgType If successful, returns the MsgType of the unpacked message
 *
 *  @return  0==success, -1==destination callsign problem, -2==source callsign problem
 *
 * Reference:  https://wsjt.sourceforge.io/FT4_FT8_QEX.pdf
 *
 **/
int unpack_type1(const uint8_t *a77, uint8_t i3, char *field1, char *field2, char *field3, MsgType *msgType) {
    uint32_t n28a, n28b;
    uint16_t igrid4;
    uint8_t ir;

    // Extract packed fields
    // read(c77,1000) n28a,ipa,n28b,ipb,ir,igrid4,i3
    // 1000 format(2(b28,b1),b1,b15,b3)
    n28a = (a77[0] << 21);  // Destination callsign
    n28a |= (a77[1] << 13);
    n28a |= (a77[2] << 5);
    n28a |= (a77[3] >> 3);
    n28b = ((a77[3] & 0x07) << 26);  // Source callsign
    n28b |= (a77[4] << 18);
    n28b |= (a77[5] << 10);
    n28b |= (a77[6] << 2);
    n28b |= (a77[7] >> 6);
    ir = ((a77[7] & 0x20) >> 5);
    igrid4 = ((a77[7] & 0x1F) << 10);
    igrid4 |= (a77[8] << 2);
    igrid4 |= (a77[9] >> 6);

    // Unpack destination station's callsign
    if (unpack28(n28a >> 1, n28a & 0x01, i3, field1) < 0) {
        return -1;
    }

    // Unpack source station's callsign
    if (unpack28(n28b >> 1, n28b & 0x01, i3, field2) < 0) {
        return -2;
    }
    // Fix "CQ_" to "CQ " -> already done in unpack28()

    // Debugging
    // DPRINTF("field1=%s field2=%s\n", field1, field2);

    // TODO: add to recent calls. (This is needed for non-standard, hashed calls)
    // if (field1[0] != '<' && strlen(field1) >= 4) {
    //     save_hash_call(field1)
    // }
    // if (field2[0] != '<' && strlen(field2) >= 4) {
    //     save_hash_call(field2)
    // }

    // Check igrid4 for a 4-char locator in G15  See reference, Table 2.
    // DPRINTF("igrid4=%u\n",igrid4);
    if (igrid4 <= MAXGRID4) {
        // DPRINTF("igrid4=%u, MAXGRID4=%u\n",igrid4, MAXGRID4);
        //  Extract 4 symbol grid locator into field3
        char *dst = field3;
        uint16_t n = igrid4;
        if (ir > 0) {
            // In case of ir=1 add an "R" before grid
            dst = stpcpy(dst, "R ");  // Does this get overwritten below???
        }

        // Code the maidenhead locator gridsquare in dst[]
        dst[4] = '\0';
        dst[3] = '0' + (n % 10);
        n /= 10;
        dst[2] = '0' + (n % 10);
        n /= 10;
        dst[1] = 'A' + (n % 18);
        n /= 18;
        dst[0] = 'A' + (n % 18);
        // if(msg(1:3).eq.'CQ ' .and. ir.eq.1) unpk77_success=.false.
        // if (ir > 0 && strncmp(field1, "CQ", 2) == 0) return -1;

        // We received a locator message.  The 4-char grid square is in field3[].  Assume Locator message type.
        *msgType = MSG_LOC;
        // DTRACE();

        // Decode igrid4 when it's an RRR, RR73, 73, blank or signal report
    } else {
        int irpt = igrid4 - MAXGRID4;  // See Reference Appendix A discussion of g15

        // Assume this is a signal report
        *msgType = MSG_RSL;
        // DPRINTF("irpt=%u\n",irpt);

        // Check special cases first
        if (irpt == 1)
            field3[0] = '\0';
        else if (irpt == 2) {
            strcpy(field3, "RRR");
            *msgType = MSG_RRR;
        }  // Roger received report???
        else if (irpt == 3) {
            strcpy(field3, "RR73");
            *msgType = MSG_RR73;
        }  // Roger received, best regards
        else if (irpt == 4) {
            strcpy(field3, "73");  // Best regards
            *msgType = MSG_73;
        } else if (irpt >= 5) {
            char *dst = field3;
            *msgType = MSG_RSL;       // It's a signal report (RSL)
            if (ir > 0) {             // Or perhaps an RRSL?
                *dst++ = 'R';         // Add "R" before report
                *msgType = MSG_RRSL;  // Yes, Roger RSL
            }
            // Extract signal report as a two digit number with a + or - sign
            int_to_dd(dst, irpt - 35, 2, true);  // signal report
        }
        // if(msg(1:3).eq.'CQ ' .and. irpt.ge.2) unpk77_success=.false.
        // if (irpt >= 2 && strncmp(field1, "CQ", 2) == 0) return -1;
    }
    // DTRACE();
    // If the destination callsign is "CQ" then change msgType to MSG_CQ
    if (strcmp(field1, "CQ") == 0) *msgType = MSG_CQ;  // CQ overides anything else we discovered

    return 0;  // Success

}  // unpack_type1()

/**
 * Unpack the FT8 13 char "free" text message
 *
 * @param a71 The packed text
 * @param text Unpacked text
 * @return 0==success
 **/
int unpack_text(const uint8_t *a71, char *text) {
    // TODO: test
    uint8_t b71[9];

    uint8_t carry = 0;
    for (int i = 0; i < 9; ++i) {
        b71[i] = carry | (a71[i] >> 1);
        carry = (a71[i] & 1) ? 0x80 : 0;
    }

    char c14[14];
    c14[13] = 0;
    for (int idx = 12; idx >= 0; --idx) {
        // Divide the long integer in b71 by 42
        uint16_t rem = 0;
        for (int i = 0; i < 9; ++i) {
            rem = (rem << 8) | b71[i];
            b71[i] = rem / 42;
            rem = rem % 42;
        }
        c14[idx] = charn(rem, 0);
    }

    strcpy(text, trim(c14));
    return 0;  // Success
}  // unpack_text()

/**
 * @brief Unpack the FT8 "telemetry" message
 * @param a71
 * @param telemetry
 * @return 0==success
 */
int unpack_telemetry(const uint8_t *a71, char *telemetry) {
    uint8_t b71[9];

    // Shift bits in a71 right by 1
    uint8_t carry = 0;
    for (int i = 0; i < 9; ++i) {
        b71[i] = (carry << 7) | (a71[i] >> 1);
        carry = (a71[i] & 0x01);
    }

    // Convert b71 to hexadecimal string
    for (int i = 0; i < 9; ++i) {
        uint8_t nibble1 = (b71[i] >> 4);
        uint8_t nibble2 = (b71[i] & 0x0F);
        char c1 = (nibble1 > 9) ? (nibble1 - 10 + 'A') : nibble1 + '0';
        char c2 = (nibble2 > 9) ? (nibble2 - 10 + 'A') : nibble2 + '0';
        telemetry[i * 2] = c1;
        telemetry[i * 2 + 1] = c2;
    }

    telemetry[18] = '\0';
    return 0;
}

/**
 * @brief Unpack the FT8 "nonstandard" messaage (with hashed callsign)
 * @author KD8CEC (for wsjtx???)
 * @param a77 The packed, non-standard message bits
 * @param field1 Unpacked first callsign
 * @param field2 Unpacked second callsign
 * @param field3 RRR, RR73, 73
 * @param msgType Sequencer's MsgType enumerations (CQ)
 */
int unpack_nonstandard(const uint8_t *a77, char *field1, char *field2, char *field3, MsgType *msgType) {
    DTRACE();

    // Assume Sequencer's MSG_UNKNOWN type for now
    *msgType = MSG_UNKNOWN;

    // Declare the FT8 tag fields per "The FT4 and FT8 Communication Protocalls" in QEX
    uint32_t h12, h1, r2, icq;
    uint64_t n58;

    // Unpack the 12-bit hash (h12) value
    h12 = (a77[0] << 4);   // 11 ~4  : 8
    h12 |= (a77[1] >> 4);  // 3~0 : 12

    // Unpack the 58-bit compressed callsign
    n58 = ((uint64_t)(a77[1] & 0x0F) << 54);  // 57 ~ 54 : 4
    n58 |= ((uint64_t)a77[2] << 46);          // 53 ~ 46 : 12
    n58 |= ((uint64_t)a77[3] << 38);          // 45 ~ 38 : 12
    n58 |= ((uint64_t)a77[4] << 30);          // 37 ~ 30 : 12
    n58 |= ((uint64_t)a77[5] << 22);          // 29 ~ 22 : 12
    n58 |= ((uint64_t)a77[6] << 14);          // 21 ~ 14 : 12
    n58 |= ((uint64_t)a77[7] << 6);           // 13 ~ 6 : 12
    n58 |= ((uint64_t)a77[8] >> 2);           // 5 ~ 0 : 765432 10

    // Unpack etc
    h1 = (a77[8] >> 1) & 0x01;     // 1==hashed callsign is second callsign, else 0
    r2 = ((a77[8] & 0x01) << 1);   // 0==blank, 1==RRR, 2==RR73, 3==73
    r2 |= (a77[9] >> 7);           //
    icq = ((a77[9] >> 6) & 0x01);  // c1 (1==first callsign is CQ and ignore h12)

    // Decompress the 58-bit callsign into c11 (with leading spaces)
    char c11[12];
    c11[11] = '\0';
    for (int i = 10; /* no condition */; --i) {
        c11[i] = charn(n58 % 38, 5);
        if (i == 0) break;
        n58 /= 38;
    }

    // Trim leading/trailing spaces from right-adjusted, uncompressed callsign
    char *uncompressedCallsign = trim(c11);

    // Save the uncompressed callsign in 12-bit hash table
    // TODO:  Do we need to confirm strlen of callsign >= 3
    FT8Hash12 err = callsignHashTable.add12(String(uncompressedCallsign));
    DPRINTF("add12(%s)=%u\n", uncompressedCallsign, err);

    // Decode the other callsign from the 12-bit hash table
    char unhashedCallsign[15];                 // Allow room for 12-char callsign, 2 angle brackets, and the NUL
    String s = callsignHashTable.lookup(h12);  // Map the 12-bit hash key to its associated callsign string
    if (s.length() != 0) {
        s = String("<") + s + String(">");  // Enclose found callsign in angle brackets
    } else {
        s = "<...>";  // Report an unknown hash key
    }
    strlcpy(unhashedCallsign, s.c_str(), sizeof(unhashedCallsign));  // Legacy code expects char[]
    DPRINTF("lookup(%u)='%s'\n", h12, unhashedCallsign);
    // unhashedCallsign[0] = '<';
    // int_to_dd(unhashedCallsign + 1, h12, 4, true);
    // unhashedCallsign[5] = '>';
    // unhashedCallsign[6] = '\0';

    // Possibly flip them around
    char *call_1 = (h1) ? uncompressedCallsign : unhashedCallsign;
    char *call_2 = (h1) ? unhashedCallsign : uncompressedCallsign;

    // Unpack the message type
    if (icq == 0) {
        *msgType = MSG_BLANK;          // Assume blank (TODO:  Sequencer may choke)
        strcpy(field1, trim(call_1));  // Destination station's callsign
        if (r2 == 1) {
            strcpy(field3, "RRR");
            *msgType = MSG_RRR;
        } else if (r2 == 2) {
            strcpy(field3, "RR73");
            *msgType = MSG_RR73;
        } else if (r2 == 3) {
            strcpy(field3, "73");
            *msgType = MSG_73;
        } else {
            field3[0] = '\0';      // No, it's just one station calling another (e.g. "W1AW KQ7B") without extra info
            *msgType = MSG_BLANK;  // Report blank (TODO:  Sequencer may choke)
        }
    } else {
        *msgType = MSG_CQ;  // It's a CQ message from a hashed callsign sans locator (TODO:  Sequencer must handle no locator)
        strcpy(field1, "CQ");
        field3[0] = '\0';  // Non-standard CQ messages do not include a locator
    }
    strcpy(field2, trim(call_2));  // Origin station's callsign

    DPRINTF("Unpacked nonstandard '%s' '%s' '%s' msgType=%u\n", field1, field2, field3, *msgType);

    return 0;
}

/**
 * Unpack a 77-bit received FT8 message
 *
 * @param a77[] The packed 77-bit FT8 message
 * @param field1 Unpacked field1, See Reference.  Needs to reference a 19 element char array for telemetry.
 * @param field2 Unpacked field2, See Reference
 * @param field3 Unpacked field3, See Reference
 *
 * @return 0==Success, -1==Error
 *
 * This is where we finally learn what (callsigns, location, signal reports, etc) is in the received message
 *
 * Note:  FT8 encodes a message into a tightly packed 77-bit string.  The i3.n3 variables
 * extracted from the message define the message types (See Reference Table-1).  The
 * meaning of the three unpacked fields depends upon the message type:
 *
 *  + 0.0 Free Text:  Unpacked into field1[]
 *  + 0.5 Telemetry:  Unpacked into field1[]
 *  + 1.  Standard Message:   field1 <- Destination callsign or CQ
 *  +                         field2 <- Source callsign
 *  +                         field3 <- 4-char locator or RRR, RR73, 73 or blank (See Reference)
 *
 * Reference:  https://wsjt.sourceforge.io/FT4_FT8_QEX.pdf
 *
 **/
int unpack77_fields(const uint8_t *a77, char *field1, char *field2, char *field3, MsgType *msgType) {
    uint8_t n3, i3;

    // Extract n3 (bits 71..73) and i3 (bits 74..76)
    n3 = ((a77[8] << 2) & 0x04) | ((a77[9] >> 6) & 0x03);
    i3 = (a77[9] >> 3) & 0x07;

    // Initial assumptions about the result
    field1[0] = field2[0] = field3[0] = '\0';
    *msgType = MSG_UNKNOWN;

    // See Reference Table-1 for explanation of i3 and n3 values.  We are here checking for Type 0.0 (13 char free text)
    if (i3 == 0 && n3 == 0) {
        int result = unpack_text(a77, field1);  // 13-character "Free Text" --> field1, field2&field3 remain NUL.
        if (result == 0) *msgType = MSG_FREE;
        return result;
    }

    // These types are not currently implemented...
    //  else if (i3 == 0 && n3 == 1) {
    //      // 0.1  K1ABC RR73; W9XYZ <KH1/KH7Z> -11   28 28 10 5       71   DXpedition Mode
    //  }
    //  else if (i3 == 0 && n3 == 2) {
    //      // 0.2  PA3XYZ/P R 590003 IO91NP           28 1 1 3 12 25   70   EU VHF contest
    //  }
    //  else if (i3 == 0 && (n3 == 3 || n3 == 4)) {
    //      // 0.3   WA9XYZ KA1ABC R 16A EMA            28 28 1 4 3 7    71   ARRL Field Day
    //      // 0.4   WA9XYZ KA1ABC R 32A EMA            28 28 1 4 3 7    71   ARRL Field Day
    //  }

    // Check for 0.5, telemetry type (does anyone use this?)
    else if (i3 == 0 && n3 == 5) {
        // 0.5   0123456789abcdef01                 71               71   Telemetry (18 hex)
        int result = unpack_telemetry(a77, field1);  // field1 gets 18 chars of hex data
        if (result == 0) *msgType = MSG_TELE;

        // Check for standard message and EU VHF contest types (maybe somebody will use this code in VHF someday)
    } else if (i3 == 1 || i3 == 2) {
        // Type 1 (standard message) or Type 2 ("/P" form for EU VHF contest)
        return unpack_type1(a77, i3, field1, field2, field3, msgType);  // Ordinary messages likely unpack here
    }

    // else if (i3 == 3) {
    //     // Type 3: ARRL RTTY Contest
    // }
    else if (i3 == 4) {
        DTRACE();
        //     // Type 4: Nonstandard calls, e.g. <WA9XYZ> PJ4/KA1ABC RR73
        //     // One hashed call or "CQ"; one compound or nonstandard call with up
        //     // to 11 characters; and (if not "CQ") an optional RRR, RR73, or 73.
        return unpack_nonstandard(a77, field1, field2, field3, msgType);
    }

    // else if (i3 == 5) {
    //     // Type 5: TU; W9XYZ K1ABC R-09 FN             1 28 28 1 7 9       74   WWROF contest
    // }

    // unknown type, should never get here.  msgType is MSG_UNKNOWN.
    return -1;
}

// Appears to be dead code?
//
//  int unpack77(const uint8_t *a77, char *message) {
//    char field1[14];
//    char field2[14];
//    char field3[7];
//    MsgType msgType;
//    int rc = unpack77_fields(a77, field1, field2, field3, &msgType);
//    if (rc < 0) return rc;

//   char *dst = message;
//   // int msg_sz = strlen(field1) + strlen(field2) + strlen(field3) + 2;

//   dst = stpcpy(dst, field1);
//   *dst++ = ' ';
//   dst = stpcpy(dst, field2);
//   *dst++ = ' ';
//   dst = stpcpy(dst, field3);
//   *dst = '\0';

//   return 0;
// }
