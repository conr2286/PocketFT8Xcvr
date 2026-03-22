/**
 * TEST
 *  + This test confirms the Si5351 clock's basic functionality
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *  + The clock's I2C bus supports communication with the Si5351
 *
 * EXERCISED
 *  + Confirms the Si5351 responds to configuration commands
 *
 * USAGE
 *  pio test -vvv -f "hw/test_03"       // Execute just this test
 *  pio test -vvv -f "hw/*"             // Execute all hw tests
 *
 * NOTES
 *  + This test verifies the Si5351 device appears on I2C? at the expected address
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "hwdefs.h"
#include "si5351.h"

// Define the I2C parameters for the Si5351
#define I2CBUS WIRE_ETC              // Which Teensy I2C bus hosts the Si5351
#define I2CADR SI5351_BUS_BASE_ADDR  // Si5351 address on I2CBUS

/**
 * @brief Helper function to read the specified Si5351 register
 * @param reg Specifies which Si5351 register to read
 * @return Bits read from register or zero if failure
 */
uint8_t si5351Read(uint8_t reg) {
    uint8_t result = 0;

    // Specify which Si5351 register
    I2CBUS.beginTransmission(I2CADR);
    I2CBUS.write(reg);
    I2CBUS.endTransmission();

    // Read Si5351 register
    I2CBUS.requestFrom(I2CADR, 1);
    while (I2CBUS.available()) {
        result = I2CBUS.read();
    }

    return result;
}  // si5351Read()

void setUp(void) {
}

void tearDown(void) {
}

/**
 * @brief Verify the Si5351 is responding at I2CADR on I2CBUS
 */
void test_si5351_address(void) {
    I2CBUS.beginTransmission(I2CADR);              // Attempt to communicate with Si5351 on I2CBUS
    unsigned response = I2CBUS.endTransmission();  // Check response
    TEST_ASSERT_EQUAL_INT(0, response);            // 0==>Si5351 responding at I2CADR on I2CBUS
}  // test_si5351_address()

/**
 * @brief Read and verify the Si5351 can initializate itself
 */
void test_si5351_init(void) {
    int msWaiting = 0;   // Milliseconds we've waited for initialization to complete
    uint8_t status = 0;  // Status register bits

    // Wait upto 1000 MS for Si5351 to complete its initialization
    status = si5351Read(SI5351_DEVICE_STATUS);  // Read status register bits
    while (msWaiting < 1000) {
        if ((status & 0x80) == 0) break;  // Finished Si5351 initialization?
        delay(10);                        // No, wait a moment
        msWaiting += 10;                  // A moment's waiting
    }

    // Check Si5351 status register bits
    TEST_ASSERT_EQUAL_INT(0, status & 0x10);  // Check for missing CLKIN signal
    TEST_ASSERT_EQUAL_INT(0, status & 0x80);  // Confirm initialization complete

}  // test_si5351_init()

/**
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_si5351_address);
    RUN_TEST(test_si5351_init);
    return UNITY_END();
}

void setup() {
    // Initialization
    Serial.begin(9600);  // Test message output device
    I2CBUS.begin();      // I2C bus hosting the Si5351

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}