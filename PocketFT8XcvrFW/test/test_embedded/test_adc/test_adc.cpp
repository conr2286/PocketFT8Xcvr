/**
 * test_adc --- Explore use of the Teensy 4.1 internal ADC for touchscreen
 *
 * Note:  This test requires Version 4.X hardware having Schematic signals
 * XD- connected to Teensy analog PIN_XDM, and YD+ connected to Teensy
 * analog PIN_YDP.  Version 2.X and 3.X hardware used an external ADC for
 * reading these touchscreen signals.
 *
 */

#include <Arduino.h>
#include "unity.h"
#include <ADC.h>
#include <ADC_Module.h>

#include "hwdefs.h"  //Schematic connections to Teensy pins

/**
 * Build a https://github.com/pedvide/ADC library object for accessing Teensy 4.1's second ADC
 *
 * ACHTUNG!  The Teensy 4.1 MCU has two ADC converters.  The Arduino libraries (e.g. analogRead)
 * (for the most part) and PJRC's AudioInputAnalog use only the first ADC which they refer to
 * as ADC1.  The Teensy 4.1 https://github.com/pedvide/ADC library provides access to the second
 * ADC which it refers to as adc1 (as the first is known here as adc0).  And, for your
 * maximum enjoyment, most https://github.com/pedvide/ADC examples access both ADCs through
 * class ADC whose constructor initializes *both*, conflicting with PJRC's Teensy 4.1
 * AudioInputAnalog class which has its own initialization for the first ADC.  :|
 *
 * The conflict is avoided here by accessing adc1 (AKA ADC2:) through the ADC_Module
 * class.  Some confusion could be avoided if we submitted a pull request to
 * https://github.com/pedvide/ADC containing an ADC constructor to initialize only one
 * ADC while leaving the other untouched.  But that would take months if not years to
 * filter into PJRC's distribution.
 */

ADC_Module secondADC(1, ADC::channel2sc1aADC1, ADC1_START);  // Access to (only) the second ADC

class TSPoint {
   public:
    int x, y, z;
    TSPoint(int x, int y, int z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

/**
 * test_unity --- Confirm successful unity installation
 */
void test_unity(void) {
    TEST_ASSERT(true);
}

/**
 * @brief Test simple reads using ADC::analogRead()
 *
 * @note This test exercises A16 and A17, each through Teensy 4.1 adc1.  Be aware
 * that the Teensy Audio Library uses adc0 (only), and we must avoid disturbing
 * adc0 here.
 */
void test_simpleReads(void) {
    // Check A16 (PIN_XDM) through ADC1
    int x1 = secondADC.analogRead(A16);          // Blocking read of A16 using ADC_0
    TEST_ASSERT_NOT_EQUAL(x1, ADC_ERROR_VALUE);  // Error?
    Serial.printf("x1=%d\n", x1);

    // Check A17 (PIN_YDP) through ADC1
    int x2 = secondADC.analogRead(A17);          // Blocking read of A16 using ADC_0
    TEST_ASSERT_NOT_EQUAL(x2, ADC_ERROR_VALUE);  // Error?
    Serial.printf("x2=%d\n", x2);
}

/**
 * @brief Test unblocked reads from adc1 (the second ADC)
 */
void test_unblockedRead(void) {
    // Start a conversion
    bool ok = secondADC.startSingleRead(A16);  // Starts the ADC without awaiting the result
    TEST_ASSERT_TRUE(ok);

    // Wait for the conversion to complete
    unsigned long us = 0;  // Let's time how long it takes
    while (!secondADC.isComplete()) {
        delayMicroseconds(1);
        us += 1;
    }
    Serial.printf("us=%ld\n", us);

    // Read conversion result
    int result = secondADC.readSingle();
    Serial.printf("result=%d\n", result);
    TEST_ASSERT(result != 0);  // There should be some noise on the pin
}

/**
 * @brief Read touchscreen event
 * @return x/y/z coordinates of touchscreent event
 */
TSPoint getPoint() {
    // Float the hardware X-Axis (which is y-Axis of landscape rotation)
    pinMode(PIN_XM, INPUT);   // Float
    pinMode(PIN_XDP, INPUT);  // Float

    // Bias the hardware Y-Axis (y-Axis of landscape rotation) with Vcc
    pinMode(PIN_YDM, OUTPUT);    // Driven
    pinMode(PIN_YP, OUTPUT);     // Driven
    digitalWrite(PIN_YP, HIGH);  // Vcc
    digitalWrite(PIN_YDM, LOW);  // Gnd

    // Wait for sample-and-hold to settle
    delayMicroseconds(20);

    // Loop waits for valid touch event
    bool valid = false;
    int xAvg, yAvg, z;  // Landscape x and y

    // Prompt operator
    // Serial.printf("Awaiting touch event\n");

    // Take two readings from the ADC
    int x1 = secondADC.analogRead(A16);
    int x2 = secondADC.analogRead(A16);
    // Serial.printf("x1=%d x2=%d\n", x1, x2);

    // Determine if these are valid (similar) readings or just noise
    if (abs(x1 - x2) < 4) {
        valid = true;          // Valid readings are similar
        xAvg = (x1 + x2) / 2;  // Average the two valid readings

        // Was this a touch event
        if (xAvg < 100) {
            valid = false;  // Discard if not a touch event
        } else {
            // Serial.printf("xAvg=%d\n", xAvg);
        }
    } else {
        valid = false;
    }

    // Setup to read yCoord from rotated (landscape) screen
    pinMode(PIN_YP, INPUT);    // Float Y-Axis pins
    pinMode(PIN_YDM, INPUT);   // Float Y-Axis pins
    pinMode(PIN_XDP, OUTPUT);  // Bias the X-Axis pins
    pinMode(PIN_XM, OUTPUT);   // Bias the X-Axis pins

    // Bias the hardware xAxis
    digitalWrite(PIN_XM, HIGH);  // Bias X-Axis with Vcc
    digitalWrite(PIN_XDP, LOW);  // Bias X-Axis with Vcc
    delayMicroseconds(20);       // Fast ARM chips need to allow voltages to settle

    // Take two readings from the ADC
    int y1 = secondADC.analogRead(A17);
    int y2 = secondADC.analogRead(A17);
    // Serial.printf("y1=%d y2=%d\n", y1, y2);

    // Determine if these are valid (similar) readings or just noise
    if (abs(y1 - y2) < 4) {
        yAvg = (y1 + y2) / 2;  // Average the two valid readings

        // Was this a touch event
        if (yAvg < 100) {
            valid = false;  // Discard if not a touch event
        } else {
            // Serial.printf("yAvg=%d\n", yAvg);
        }
    } else {
        valid = false;
    }

    if (!valid) {
        z = 0;
    } else {
        z = xAvg + yAvg;
    }
    return TSPoint(xAvg, yAvg, z);
}

/**
 * @brief test reading touch Xcoord via ADC library
 *
 * @note The touchscreen is rotated 90 degrees.  So... to read an xCoord, we have
 * to float the hardware xAxis, bias the hardware yAxis, and read the xAxis.
 */
void test_touch(void) {
    // Loop searching for maxX and maxY coordinate values
    int maxX = 0, maxY = 0;
    int minX = 9999, minY = 9999;

    for (int i = 1; i <= 2000; i++) {
        // Read touch point
        TSPoint p = getPoint();

        // Valid touch event?
        if (p.z > MINPRESSURE) {
            // Test for new max values
            // Serial.printf("x=%d y=%d z=%d\n", p.x, p.y, p.z);
            if (p.x > maxX) maxX = p.x;
            if (p.x < minX) minX = p.x;
            if (p.y > maxY) maxY = p.y;
            if (p.y < minY) minY = p.y;

            // Covert ADC readings into screen coordinates
            p.x = map(p.x, 137, 2182, 0, 480);  // Map to resistance to screen coordinate
            p.y = map(p.y, 126, 1392, 0, 320);
        }

        // Display max values
        Serial.printf("minX=%d maxX=%d, minY=%d maxY=%d\n", minX, maxX, minY, maxY);

        // Wait before next read
        delay(10);
    }

    // Assert min/max on x/y axis
    TEST_ASSERT(maxX >= 137);
    TEST_ASSERT(maxX <= 2182);
    TEST_ASSERT(maxY >= 126);
    TEST_ASSERT(maxY <= 1392);
}

/**
 * @brief Function to execute each test case
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_unity);          // Confirm working unity installation
    RUN_TEST(test_simpleReads);    // Simple reads using analogRead()
    RUN_TEST(test_unblockedRead);  // Non-blocking read
    RUN_TEST(test_touch);          // touch X-Coord
    return UNITY_END();
}

/**
 * @brief This is the unity setUp() function executed prior to each test case
 */
void setUp(void) {
}

/**
 * @brief This is the Arduino setup() function initializes everything before tests begin
 */
void setup() {
    // Get USB Serial running
    Serial.begin(9600);

    // Config second ADC for 12-bit resolution
    secondADC.setResolution(12);

    // Execute the tests
    runUnityTests();
}

/**
 * @brief Teardown following each test case
 */
void tearDown(void) {
}

/**
 * @brief Arduino loop() is not actually used here
 */
void loop() {
}