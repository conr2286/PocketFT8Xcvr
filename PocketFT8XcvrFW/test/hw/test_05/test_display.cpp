/**
 * TEST
 *  + This test exercises the Adafruit 3.5" TFT 320x480 touchscreen display breakout board
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *
 * EXERCISED
 *  + Connectivity with the Adafruit 3.5" Product 2050 display
 *  + Display orientation
 *  + Display coordinate system
 *  + Connectivity with the Adafruit 3.5" Product 2050 touchscreen
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
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen

// Build the drivers for the Adafruit display and touchscreen
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
TouchScreen ts = TouchScreen(PIN_XDP, PIN_YP, PIN_XM, PIN_YDM, 282);                  // The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024

void setUp(void) {
}

void tearDown(void) {
}

/**
 * @brief Exercise
 */
void test_display_communication(void) {
}  // test_config()

/**
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_display_communication);
    return UNITY_END();
}

void setup() {
    // Initialization
    Serial.begin(9600);  // Test message output device

    // Initialize the display
    tft.begin(30000000UL, 2000000UL);  // Configure SPI clock speeds
    tft.setRotation(3);                // Configure landscape screen rotation
    tft.fillScreen(HX8357_BLACK);      // Erase the display

    // Run the tests
    runUnityTests();
}

void loop() {
}