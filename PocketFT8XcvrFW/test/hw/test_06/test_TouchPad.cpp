/**
 * TEST
 *  + Exercise the Adafruit 320x480 resistive touch pad (P2050)
 *
 * EXERCISED
 *  + Teensy MCU:  Must be able to load and execute this test program
 *  + Adafruit 320x480 TFT display (P2050)
 *  + Adafruit 320x480 resistive touch pad (P2050)
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <HX8357_t3n.h>
#include <DEBUG.h>

#include "hwdefs.h"
#include "FT8Font.h"  //Customized font for the Pocket FT8 Revisited

#include "TouchPad.h"

static const unsigned ADC_SETTLE_US = 20;

// Build the Adafruit display driver for Teensy
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

// Build the uncalibrated TouchPad driver
TouchPad touchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

/**
 * @brief Determine if a is approximately equal to b
 * @param a value1
 * @param b value2
 * @param tolerance allowed difference
 * @return true if a lies within the allowed difference of b
 */
bool isNear(int a, int b, int tolerance) {
    DPRINTF("a=%d b=%d tolerance=%d\n", a, b, tolerance);
    if ((a >= b - tolerance) && (a <= b + tolerance)) return true;
    return false;
}  // isNear()

/**
 * @brief Helper function to configure GPIO pin to float
 * @param pin GPIO pin number
 *
 * @note Floating a pin removes the digital I/O circuitry from that pin
 */
void floatPin(unsigned pin) {
    pinMode(pin, INPUT_DISABLE);  // Floats a Teensy 4.1 GPIO pin
}  // floatPin()

/**
 * @brief Helper function to "ground" a pin
 * @param pin GPIO pin number
 *
 * @note The specified pin is brought to logic level zero, close to but not
 * truly ground.
 */
void groundPin(unsigned pin) {
    pinMode(pin, OUTPUT);    // Configure the pin for digital output
    digitalWrite(pin, LOW);  // And a logic zero
}  // groundPin()

/**
 * @brief Helper function to supply Vcc to a pin
 * @param pin GPIO pin number
 *
 * @note The specified pin is actually supplied with logic level one, close to but
 * not truly Vcc.
 */
void vccPin(unsigned pin) {
    pinMode(pin, OUTPUT);     // Configure this pin for digital output
    digitalWrite(pin, HIGH);  // Supply logic one to pin
}  // vccPin()

/**
 * @brief Display prompt message
 * @param msg The message String
 */
void displayPromptMsg(String msg) {
    tft.fillScreen(HX8357_BLACK);
    tft.setCursor(20, 20);
    tft.println("test_TouchScreen");
    tft.setCursor(20, 40);
    tft.println(msg);
    delay(50);
}

/**
 * @brief This is the Unity setup() function invoked once before each test case
 * @param
 */
void setUp(void) {
    // Erase the display
}

/**
 * @brief Function invoked once following each test case
 *
 * @brief Floats all the touchscreen pins
 */
void tearDown(void) {
    floatPin(PIN_XM);
    floatPin(PIN_XP);
    floatPin(PIN_XR);
    floatPin(PIN_YM);
    floatPin(PIN_XP);
    floatPin(PIN_YR);
}

/**
 * @brief This test exercises electrical connections between touchpad and Teensy MCU
 */
void test_wiring(void) {
    int xm, yp;

    // Prompt operator
    displayPromptMsg("Do not touch screen");

    // Check grounded X-Axis
    groundPin(PIN_XP);                 // X+
    groundPin(PIN_XR);                 // 510 Ohm resistor
    delayMicroseconds(ADC_SETTLE_US);  // Allow analog signals to settle
    xm = analogRead(PIN_XM);           // Raw ADC value ranges 0..1023
    DPRINTF("xm=%u\n", xm);
    TEST_ASSERT(isNear(xm, 0, 20));  // X- Should be near ground

    // Check grounded Y-Axis
    groundPin(PIN_YM);                 // Y-
    groundPin(PIN_YR);                 // 510 Ohm resistor
    delayMicroseconds(ADC_SETTLE_US);  // Allow analog signals to settle
    yp = analogRead(PIN_YP);           // Raw ADC value ranges 0..1023
    TEST_ASSERT(isNear(yp, 0, 20));    // Y+ should be near ground

    // Check  X-Axis at Vcc
    vccPin(PIN_XP);                     // X+
    vccPin(PIN_XR);                     // 510 Ohm resistor
    delayMicroseconds(ADC_SETTLE_US);   // Allow analog signals to settle
    xm = analogRead(PIN_XM);            // Raw ADC value ranges 0..1023
    TEST_ASSERT(isNear(xm, 1023, 20));  // X- Should be near Vcc

    // Check Y-Axis at Vcc
    vccPin(PIN_YM);                     // Y-
    vccPin(PIN_YR);                     // 510 Ohm resistor
    delayMicroseconds(ADC_SETTLE_US);   // Allow analog signals to settle
    yp = analogRead(PIN_YP);            // Raw ADC value ranges 0..1023
    TEST_ASSERT(isNear(yp, 1023, 20));  // Y+ should be near Vcc
}

/**
 * @brief Test a touch event
 *
 * DISCUSSION:
 *  + We display the filtered ADC results but don't really pay much attention to them
 */
void test_touchPad(void) {
    displayPromptMsg("Touch the screen");

    // Wait for operator to touch the screen
    TouchPadPoint result;
    while (!touchPad.readFiltered(result)) {
        delay(50);  // Wait a moment
    }

    // Display filtered ADC values from result
    tft.setCursor(20, 60);
    tft.printf("xADC=%u yADC=%u", result.x, result.y);

    // ADC results should lie within 0..1023 non-inclusive
    TEST_ASSERT_GREATER_THAN_INT32(0, result.x);
    TEST_ASSERT_GREATER_THAN_INT32(0, result.y);
    TEST_ASSERT_LESS_THAN_INT32(1023, result.x);
    TEST_ASSERT_LESS_THAN_INT32(1023, result.y);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wiring);
    RUN_TEST(test_touchPad);
    return UNITY_END();
}

/**
 * @brief This is the Arduino setup() function invoked once only
 */
void setup() {
    delay(10);
    Serial.begin(9600);
    delay(10);
    Serial.println("Starting...");

    // Setup the Adafruit display and graphics library for use by our application
    tft.begin(30000000UL, 2000000UL);  // Configure SPI clock speeds for Teensy 4.1
    tft.setRotation(1);                // Configure screen rotation
    tft.setFont(&FT8Font);             // Configure the font
    tft.setTextColor(HX8357_YELLOW);   // Yellow
    tft.setTextSize(1);                // Not too small

    // Run the tests
    runUnityTests();

}  // setup()

void loop() {
    // delay(1000);
}