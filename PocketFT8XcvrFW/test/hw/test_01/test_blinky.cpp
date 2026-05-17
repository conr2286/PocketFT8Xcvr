/**
 * TEST
 *  + A very basic test to be executed on a non-smoking, new board
 *
 * EXERCISED
 *  + Teensy MCU:  Must be able to load and execute this test program
 *  + XMIT LED:  This test blinks the XMIT LED for about 10 seconds
 *
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "hwdefs.h"

void setUp(void) {
}

void tearDown(void) {
}

/**
 *  Exercise the XMIT LED
 */
void test_xmit_led(void) {
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
}  // test_xmit_led

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_xmit_led);
    return UNITY_END();
}

void setup() {
    delay(10);
    Serial.begin(9600);
    delay(10);
    Serial.println("Watch the blinking XMIT LED");

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}