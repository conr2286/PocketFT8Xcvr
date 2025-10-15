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
    TEST_ASSERT(table.add("") == FT8CallsignHashTable::error);              // Zero length callsign
    TEST_ASSERT(table.add("123456789ABC") == FT8CallsignHashTable::error);  // More than 11 characters
    TEST_ASSERT(table.add("ABC!!!") == FT8CallsignHashTable::error);        // Illegal callsign characters
}

/**
 * @brief Confirm hashing valid callsigns returns a valid hash key (not error)
 */
void test_valid_callsigns(void) {
    TEST_ASSERT(table.add("W1AW/P1") != FT8CallsignHashTable::error);    // Valid callsign
    TEST_ASSERT(table.add("l0wercase") != FT8CallsignHashTable::error);  // Lower case callsign
    TEST_ASSERT(table.add("W1AW/P2 ") != FT8CallsignHashTable::error);   // Valid callsign with trailing blank
    TEST_ASSERT(table.add(" W1AW/P3") != FT8CallsignHashTable::error);   // Valid callsign with leading blank
}

/**
 * @brief Confirm strings are hashed to their FT8-expected 22-bit key values
 */
void test_hash_values(void) {
    TEST_ASSERT_EQUAL_UINT32(table.add("KQ7B/IDAHO"), 21709L);    // An expected 22-bit key value
    TEST_ASSERT_EQUAL_UINT32(table.add("ABCDEFGHIJK"), 2152258);  // Exercise the longest allowed nonstandard call
    TEST_ASSERT_EQUAL_UINT32(table.add("A0Z"), 1925787);          // Exercise the shortest nonstandard call we might realisticly encounter
    TEST_ASSERT_EQUAL_UINT32(table.add("aa0aaa"), 2304112);       // Verify conversion of lower to uppercase before hashing
    TEST_ASSERT_EQUAL_UINT32(table.add("W1AW/7"), 3652365);       // Verify hashing of a slash character in the call
    TEST_ASSERT_EQUAL_UINT32(table.add("KQ7B MOBILE"), 2894049);  // Verify hashing of a space character in the call
}

/**
 * @brief Confirm strings are hashed to the FT8-expected 12-bit keys
 */
void test_hash_12bit(void) {
    TEST_ASSERT_EQUAL_UINT16(table.shrinkKey12(table.add("KQ7B/UTAH")), 2112);
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

    // Finished
    UNITY_END();
}

/**
 * @brief Nothing to do in the Arduino loop() function
 */
void loop() {
    // Not used
}