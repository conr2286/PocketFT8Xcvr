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

/**
 * @brief Helper function to poll the touchscreen using ADC2
 *
 * @return TSPoint with x,y and z ("pressure")
 *
 * @note Returns z==0 if no touch reading is available
 *
 */
TSPoint pollTouchscreen() {
    TSPoint pi, pw;

    pi = ts.getPoint();  // Read the screen

    if (pi.z > MINPRESSURE) {  // Has screen been touched?
        Serial.printf("ADC = (%d,%d,%d)\n", pi.x, pi.y, pi.z);
        pw.x = map(pi.x, TS_MINX, TS_MAXX, 0, 480);  // Map to resistance to screen coordinate
        pw.y = map(pi.y, TS_MINY, TS_MAXY, 0, 320);
        pw.z = pi.z;
    } else {
        pw.z = 0;  // Invalid point (no touchpoint available)
    }
    return pw;
}

// Unity function invoked prior to each test
void setUp(void) {
}

// Unity function invoked following each test
void tearDown(void) {
    // TSPoint p1;
    // tft.drawString("Touch to continue", 140, 150);
    // unsigned long dt = 10;       // ms to wait between polls
    // unsigned long tWait = 0;     // ms we've awaited touch
    // while (tWait < 10000L) {     // We're willing to wait 10 seconds
    //     p1 = pollTouchscreen();  // Read touch input, if any
    //     if (p1.z > 100) break;   // Touch?
    //     delay(dt);               // Wait before checking again
    //     tWait += dt;             // Tally wait ms
    // }
    // if (p1.z > 100) {
    //     Serial.printf("Touch = (%d,%d)\n", p1.x, p1.y);
    // }
}  // tearDown()

/**
 * @brief Draw a rectangle and label the origin and lower-right corner
 *
 * @note There are not sensible assertions here because the HX8357 code
 * doesn't inform us when something fails, so we rely on user input
 */
void test_rect(void) {
    tft.drawRoundRect(0, 0, 480, 320, 5, HX8357_YELLOW);
    tft.drawPixel(0, 0, HX8357_WHITE);
    tft.drawString("(0,0)", 5, 8);
    tft.drawPixel(479, 319, HX8357_WHITE);
    tft.drawString("(479,319)", 340, 300);
}  // test_config()

/**
 * @brief Check touchscreen x-Axis wiring
 */
void test_yAxisPins(void) {
    int ydp;

    // Ground the Y-Axis
    pinMode(PIN_YDM, OUTPUT);
    pinMode(PIN_YP, OUTPUT);
    digitalWrite(PIN_YDM, LOW);
    digitalWrite(PIN_YP, LOW);

    // Confirm YDP near zero with y-Axis grounded
    ydp = analogRead(A17);
    Serial.printf("ydp=%d\n", ydp);

    // Confirm YDP remains near zero with YM floating and YP grounded
    for (int i = 0; i < 10; i++) {
        // Float the Y-Axis
        pinMode(PIN_YDM, INPUT_DISABLE);

        // Check YDP
        ydp = analogRead(A17);
        Serial.printf("%d: ydp=%d\n", i, ydp);

        delay(10);
    }

    // Confirm YDP remains near zero with y-Axis floating
    for (int i = 0; i < 10; i++) {
        // Float the Y-Axis
        pinMode(PIN_YDM, INPUT_DISABLE);
        pinMode(PIN_YP, INPUT_DISABLE);

        // Check YDP
        ydp = analogRead(A17);
        Serial.printf("%d: ydp=%d\n", i, ydp);

        delay(10);
    }
}

/*
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_yAxisPins);
    RUN_TEST(test_rect);
    return UNITY_END();
}

void setup() {
    delay(10);           // Wait for reliable comm with host computer when debugging
    Serial.begin(9600);  // Test message output device
    Serial.printf("Starting...\n");

    // Initialize the display
    tft.begin(30000000UL, 2000000UL);  // Configure SPI clock speeds
    tft.setRotation(3);                // Configure landscape screen rotation
    tft.fillScreen(HX8357_BLACK);      // Erase the display
    tft.setTextSize(2);

    // Discard one ADC reading
    TSPoint foo = ts.getPoint();

    // Run the tests
    runUnityTests();
}

void loop() {
}