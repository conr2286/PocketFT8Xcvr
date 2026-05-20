/**
 * TEST
 *  + Exercise the Adafruit 320x480 TFT display (P2050)
 *
 * EXERCISED
 *  + Teensy MCU:  Must be able to load and execute this test program
 *  + TFT Display
 *
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <HX8357_t3n.h>

#include "hwdefs.h"
#include "FT8Font.h"  //Customized font for the Pocket FT8 Revisited

// Build the display driver
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

void setUp(void) {
    tft.fillScreen(HX8357_BLACK);  // Erase the display
}

void tearDown(void) {
}

/**
 *  ERASE the SCREEN
 */
void test_communication(void) {
    TEST_ASSERT_EQUAL(480, tft.width());
    TEST_ASSERT_EQUAL(320, tft.height());

}  // test_communication

void test_text(void) {
    TEST_ASSERT_EQUAL(3, tft.getRotation());
    tft.setCursor(20, 20);
    tft.print("TEST");
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_communication);
    RUN_TEST(test_text);
    return UNITY_END();
}

void setup() {
    delay(10);
    Serial.begin(9600);
    delay(10);
    Serial.println("Starting...");

    // Setup the Adafruit display and graphics library for use by our application
    tft.begin(30000000UL, 2000000UL);  // Configure SPI clock speeds for Teensy 4.1
    tft.setRotation(3);                // Configure screen rotation
    tft.setFont(&FT8Font);             // Configure the font
    tft.setTextSize(2);

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}