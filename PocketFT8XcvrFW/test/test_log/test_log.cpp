#include <Arduino.h>
#include <unity.h>

#include "Contact.h"
#include "LogFactory.h"

ContactLogFile* testLog;  // The contact log file
Contact testContact;      // A contact for testing

void setUp(void) {
    testContact.begin("T3ST", "AA0A", 7074, "FT8", "Ozone Blaster III", 1, "SOTA");  // Start gathering contact info
}

void tearDown(void) {
    testContact.reset();
}

/**
 *  Creates a log entry with myRSL and workedRSL and the other fields supplied by/in begin()
 */
void test_isValid(void) {
    TEST_ASSERT(!testContact.isValid());
    testContact.setMyRSL("-1");
    testContact.setWorkedRSL("-2");
    TEST_ASSERT(testContact.isValid());
}

/**
 * @brief Test rig entry
 */
void test_rig(void) {
    TEST_MESSAGE("test_rig");
    char rig[] = "https://github.com/conr2286/PocketFT8Xcvr";
    testContact.setRig(rig);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(rig, testContact.getRig(), sizeof(rig));
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_isValid);
    RUN_TEST(test_rig);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);

    testLog = LogFactory::buildADIFlog("TESTLOG.ADIF");

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}