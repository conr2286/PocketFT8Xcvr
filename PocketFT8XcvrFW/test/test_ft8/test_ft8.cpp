/**
 * test_time exercises the ft8_lib interface
 */

#include <Arduino.h>
#include <unity.h>
#include "ft8LibIfce.h"
#include "message.h"
#include "text.h"
#include "decode_ft8.h"

/**
 * @brief This is the unity setup method executed prior to each test
 */
void setUp(void) {
}

/**
 * @brief This is the unity tearDown method executed following each test
 * @param
 */
void tearDown(void) {
}

void test_trim_brackets(void) {
    char s1[35];
    char s2[35];

    // Should leave string without brackets unchanged
    strlcpy(s1, "ABC", sizeof(s1));
    strlcpy(s2, s1, sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Should leave empty string unchanged
    strlcpy(s1, "", sizeof(s1));
    strlcpy(s2, s1, sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Should be able to trim legal string with brackets
    strlcpy(s1, "<ABC>", sizeof(s1));
    strlcpy(s2, "ABC", sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s2, s1);

    // Should be able to trim a short string
    strlcpy(s1, "<A>", sizeof(s1));
    strlcpy(s2, "A", sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Should leave empty string within brackets unchanged
    strlcpy(s1, "<>", sizeof(s1));
    strlcpy(s2, s1, sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Should be able to trim brackets from the longest allowed nonstandard callsign
    strlcpy(s1, "<AA0AAA/8901>", sizeof(s1));
    strlcpy(s2, "AA0AAA/8901", sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Excessively long strings should remain unchanged
    strlcpy(s1, "<ABCDEFGHIJKLMNOPQRSTUVWYZ>", sizeof(s1));
    strlcpy(s2, s1, sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);

    // Should be able to trim the unknown hashed callsign indicator (to what???)
    strlcpy(s1, "<...>", sizeof(s1));
    strlcpy(s2, "...", sizeof(s2));
    trimBracketsFromCallsign(s1);
    TEST_ASSERT_EQUAL_STRING(s1, s2);
}

/**
 * @brief Exercise QSO using standard messages w/o any hashed callsigns in field1 or field2
 *
 * @note The chosen test messages are derived from Section 7 "Message Sequencing" of
 * the "The FT4 and FT8 Communication Protocols" in QEX.
 */
void test_standard_message(void) {
    char textMsg[FTX_MAX_MESSAGE_LENGTH];                     // Unpacked plain text message
    uint8_t pckdMsg[FTX_PAYLOAD_LENGTH_BYTES];                // Packed FT8 message
    char field1[FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE];  // Received field1
    char field2[FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE];  // Received field2
    char field3[7];                                           // Received field3
    char tempBfr[FTX_MAX_MESSAGE_LENGTH];                     // Temporary text buffer
    MsgType msgType;                                          // Received message type

    // Exercise standard CQ message
    strcpy(textMsg, "CQ AA0AAA DN15");                                                       // TX6
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise standard station call
    strcpy(textMsg, "AA0AAA AA9AAA EN50");                                                   // TX1
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RSL
    strcpy(textMsg, "AA9AAA AA0AAA -10");                                                    // TX2
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RRSL
    strcpy(textMsg, "AA0AAA AA9AAA R-12");                                                   // TX3
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RRR
    strcpy(textMsg, "AA9AAA AA0AAA RRR");                                                    // TX4
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise 73
    strcpy(textMsg, "AA0AAA AA9AAA 73");                                                     // TX5
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text
}  // test_standard_message()

/**
 * @brief Exercise QSO using a nonstandard CQ message
 */
void test_nonstandard_cq(void) {
    char textMsg[FTX_MAX_MESSAGE_LENGTH];                     // Unpacked plain text message
    uint8_t pckdMsg[FTX_PAYLOAD_LENGTH_BYTES];                // Packed FT8 message
    char field1[FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE];  // Received field1
    char field2[FTX_NONSTANDARD_BRACKETED_CALLSIGN_BFRSIZE];  // Received field2
    char field3[7];                                           // Received field3
    char tempBfr[FTX_MAX_MESSAGE_LENGTH];                     // Temporary text buffer
    MsgType msgType;                                          // Received message type

    // Exercise nonstandard CQ message (Note:  Nonstandard CQ message omits locator)
    strcpy(textMsg, "CQ AA0AAA/IOWA");                                                       // TX6
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise call to nonstandard call
    strcpy(textMsg, "AA0AAA/IOWA AA9AAA EN50");                                              // TX1
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RSL
    strcpy(textMsg, "AA9AAA AA0AAA/IOWA -10");                                               // TX2
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RRSL
    strcpy(textMsg, "AA0AAA/IOWA AA9AAA R-12");                                              // TX3
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise RRR
    strcpy(textMsg, "AA9AAA AA0AAA/IOWA RRR");                                               // TX4
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text

    // Exercise 73
    strcpy(textMsg, "AA0AAA/IOWA AA9AAA 73");                                                // TX5
    TEST_ASSERT_EQUAL_INT32(0, pack77(textMsg, pckdMsg));                                    // Pack text into bfr
    TEST_ASSERT_EQUAL_INT32(0, unpack77_fields(pckdMsg, field1, field2, field3, &msgType));  // Unpack bfr into temp
    trimBracketsFromCallsign(field1);                                                        // Remove brackets from received callsigns
    trimBracketsFromCallsign(field2);                                                        // Remove brackets from received callsigns
    strlcpy(tempBfr, field1, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field2, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    strlcat(tempBfr, " ", sizeof(tempBfr));                                                  // Assemble received msg text in tempBfr
    strlcat(tempBfr, field3, sizeof(tempBfr));                                               // Assemble received msg text in tempBfr
    TEST_ASSERT_EQUAL_STRING(textMsg, tempBfr);                                              // Should recover orignal message text
}  // test_nonstandard_cq()

/**
 * @brief This is the Arduino setup() function invoked when program starts
 */
void setup() {
    Serial.begin(9600);
    delay(1000);

    // Initialize unity
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_trim_brackets);
    RUN_TEST(test_standard_message);
    RUN_TEST(test_nonstandard_cq);

    // Finished
    UNITY_END();
}

/**
 * @brief Nothing to do in the Arduino loop() function
 */
void loop() {
    // Not used
}