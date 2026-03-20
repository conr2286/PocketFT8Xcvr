/**
 * This test is intended for turning-on a new HW board.
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
#if HW_VERSION < 4
    TEST_ASSERT_EQUAL(2, nDevices);  // V3 boards find Si5351 and MCP342X
#else
    TEST_ASSERT_EQUAL(1, nDevices);  // V4 boards find Si5351
#endif
}  // test_Wire2()

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_xmit);
    RUN_TEST(test_Wire);
    RUN_TEST(test_Wire2);
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