/**
 * TEST
 *  + This test exercises the Adafruit 3.5" TFT 320x480 touchscreen display breakout board
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *
 * EXERCISED
 *  + Exercises connectivity with the Adafruit 3.5" Product #2050 display via
 *  + Exercises connectivity with the Adafruit 3.5" Product #2050 touchscreen
 *
 * USAGE
 *  pio test -vvv -f "hw/test_05"       // Execute just this test
 *  pio test -vvv -f "hw/*"             // Execute all hw tests
 *
 * NOTES
 *  + Verion 4 hardware communicates with the touchscreen using the Teensy 4.1's ADC2
 *  + Earlier hardware versions used an I2C connection to an external MC342x ADC
 *  + The executing test draws a rectangle around the display's borders and prompts the
 *    operator to "draw" on the screen using the touchscreen
 *  + The test ends after 10 seconds of no operator interaction (i.e. touch)
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
 * @brief Exercise Si4735 configuration
 */
void test_config(void) {
}  // test_config()

/**
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_config);
    return UNITY_END();
}

void setup() {
    // Initialization
    Serial.begin(9600);  // Test message output device

    // Run the tests
    runUnityTests();
}

void loop() {
}