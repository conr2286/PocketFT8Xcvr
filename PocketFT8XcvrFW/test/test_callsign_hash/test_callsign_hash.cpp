/**
 * test_time exercises the FT8 Callsign Hash Table
 */

#include <Arduino.h>
#include <unity.h>
#include "FT8CallsignHashTable.h"

static FT8CallsignHashTable table;

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

/**
 * @brief Confirm hashing invalid callsigns will return an error indication
 */
void test_invalid_callsigns(void) {
    TEST_ASSERT_EQUAL_UINT32(FT8CallsignHashTable::error, table.add12(""));              // Zero length callsign
    TEST_ASSERT_EQUAL_UINT32(FT8CallsignHashTable::error, table.add12("123456789ABC"));  // More than 11 characters
    TEST_ASSERT_EQUAL_UINT32(FT8CallsignHashTable::error, table.add12("ABC!!!"));        // Illegal callsign characters
}

/**
 * @brief Confirm valid callsigns aren't hashing to error
 */
void test_valid_callsigns(void) {
    TEST_ASSERT(table.add12("W1AW/P1") != FT8CallsignHashTable::error);    // Valid callsign
    TEST_ASSERT(table.add12("l0wercase") != FT8CallsignHashTable::error);  // Lower case callsign
    TEST_ASSERT(table.add12("W1AW/P2 ") != FT8CallsignHashTable::error);   // Valid callsign with trailing blank
    TEST_ASSERT(table.add12(" W1AW/P3") != FT8CallsignHashTable::error);   // Valid callsign with leading blank
}

/**
 * @brief Confirm valid strings are hashed to their FT8-expected 22-bit key values
 */
void test_hash_values(void) {
    TEST_ASSERT_EQUAL_UINT32(21709L, table.add22("KQ7B/IDAHO"));  // Test an expected 22-bit hash
    TEST_ASSERT_EQUAL_UINT32(2101L, table.add12("ABCDEFGHIJK"));  // Exercise the longest allowed nonstandard call
    TEST_ASSERT_EQUAL_UINT32(1880L, table.add12("A0Z"));          // Exercise the shortest nonstandard call we might realisticly encounter
    TEST_ASSERT_EQUAL_UINT32(2250L, table.add12("aa0aaa"));       // Verify conversion of lower to uppercase before hashing
    TEST_ASSERT_EQUAL_UINT32(3566L, table.add12("W1AW/7"));       // Verify hashing of a slash character in the call
    TEST_ASSERT_EQUAL_UINT32(2826L, table.add12("KQ7B MOBILE"));  // Verify hashing of a space character in the call
}

/**
 * @brief Confirm lookup of entries known to reside in the map
 */
void test_lookup_valid(void) {
    String callsign = "KQ7B/IC257";                              // Nonstandard callsign
    FT8Hash12 key = table.add12(callsign);                       // Record this callsign in the map
    TEST_ASSERT(key != FT8CallsignHashTable::error);             // Confirm success
    String result = table.lookup(key);                           // Map key to the associated callsign
    TEST_ASSERT_EQUAL_STRING(callsign.c_str(), result.c_str());  // Confirm retrieval of recorded callsign
}

/**
 * @brief Confirm lookup of unknown key returns an empty String
 */
void test_lookup_unknown(void) {
    String callsign = "KQ7B/JIM";                     // Nonstandard callsign
    FT8Hash12 key = table.add12(callsign);            // Record this callsign in the map
    TEST_ASSERT(key != FT8CallsignHashTable::error);  // Confirm success
    uint32_t unknownKey = 10523793;                   // Very likely an unknown key
    String result = table.lookup(unknownKey);         // Map key to the associated callsign
    TEST_ASSERT(result.length() == 0);                // Confirm failed retrieval
}

/**
 * @brief This is the Arduino setup() function invoked when program starts
 */
void setup() {
    Serial.begin(9600);
    delay(1000);

    // Initialize unity
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_invalid_callsigns);
    RUN_TEST(test_valid_callsigns);
    RUN_TEST(test_hash_values);
    RUN_TEST(test_lookup_valid);
    RUN_TEST(test_lookup_unknown);

    // Finished
    UNITY_END();
}

/**
 * @brief Nothing to do in the Arduino loop() function
 */
void loop() {
    // Not used
}