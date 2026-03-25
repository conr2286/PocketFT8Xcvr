/**
 * TEST
 *  + This test investigates and reports the addresses of the devices on I2C1 and I2C2
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *
 * EXERCISED
 *  + This test scans every possible bus address on I2C1
 *  + This test scans every possible bus address on I2C2
 *
 * USAGE
 *  pio test -vvv -f "hw/test_02"       // Execute just this test
 *  pio test -vvv -f "hw/\*"             // Execute all hw tests
 *
 * NOTES
 *  + This test reports the address of each device found on the two I2C busses.
 *  + Version 4 hardware eliminated the MCP342X ADC from Wire2 (ie. I2C2) and this test
 *    will report an error if compiled for Version 4 but executed on older hardware as
 *    it expects to find but a single device on Wire2.
 *  + This test is unable to confirm the expected devices are responding at their expected
 *    addresses as can their respective i2c device tests.
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
 * @brief Explore I2C devices on Wire1
 */
#define I2C1 Wire
void test_Wire(void) {
    // Initialization
    I2C1.begin();  // Initialize I2C bus
    delay(10);     // Give the bus a moment

    int nDevices = 0;

    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device acknowledged an address.
    for (int address = 1; address < 127; address++) {
        // Scan for device at address
        I2C1.beginTransmission(address);
        int error = I2C1.endTransmission();

        if (error == 0) {
            Serial.printf("Device found on SDA/SCL (I2C1) at 0x%02x\n", address);
            nDevices++;
        } else if (error == 4) {
            Serial.printf("Error on addr 0x%02x, errno=%d\n", address, error);
        }
    }

    // We expect to find one I2C device (Si4735) responding on the Wire bus.
    // Note:  The Si4735 is very sensitive to I2C noise from other devices.
    printf("Found %d devices on SDA/SCL (Wire)\n", nDevices);
    TEST_ASSERT_EQUAL(1, nDevices);
}  // test_Wire

/**
 * @brief Explore I2C devices on Wire2
 */
#define I2C2 Wire2
void test_Wire2(void) {
    // Initialization
    I2C2.begin();  // Initialize I2C bus
    delay(10);     // Give the bus a moment

    int nDevices = 0;

    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device acknowledged an address.
    for (int address = 1; address < 127; address++) {
        // Scan for device at address
        I2C2.beginTransmission(address);
        int error = I2C2.endTransmission();

        if (error == 0) {
            Serial.printf("Device found on SDA1/SCL1 (Wire2) at 0x%02x\n", address);
            nDevices++;
        } else if (error == 4) {
            Serial.printf("Error on addr 0x%02x, errno=%d\n", address, error);
        }
    }

    // We expect to find one I2C device (Si5351) responding on the Wire2 bus.
    printf("Found %d devices on SDA1/SCL1 (Wire2)\n", nDevices);
// Patched V3 boards need HW_VERSION 3
#define HW_VERSION 3  // Remove this definition for "real" V4 boards
#if HW_VERSION < 4
    TEST_ASSERT_EQUAL(2, nDevices);  // V3 boards find Si5351 and MCP342X
#else
    TEST_ASSERT_EQUAL(1, nDevices);  // V4 boards find only Si5351
#endif
}  // test_Wire2()

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Wire);
    RUN_TEST(test_Wire2);
    return UNITY_END();
}

void setup() {
    delay(10);
    Serial.begin(9600);
    Wire.begin();
    Wire1.begin();
    delay(100);

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}