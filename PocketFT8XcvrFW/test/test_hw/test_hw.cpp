/**
 * This test is intended for turning-on a new HW board.
 *
 */

#include <Arduino.h>
#include <unity.h>
#include "hwdefs.h"

void setUp(void) {
}

void tearDown(void) {
}

/**
 *  Exercise the XMIT LED
 */
void test_xmit(void) {
    // Configure GPIO pin
    pinMode(PIN_XMT, OUTPUT);

    // Blink XMIT LED for 10 seconds
    for (int secsRemaining = 10; secsRemaining > 0; secsRemaining--) {
        Serial.printf("%s seconds remaining=%d\n", __FUNCTION__, secsRemaining);
        digitalWrite(PIN_XMT, HIGH);
        delay(500);
        digitalWrite(PIN_XMT, LOW);
        delay(500);
    }
}  // test_xmit

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_xmit);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}