/**
 * TEST
 *  + This test confirms connectivity with the Si4735
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *  + The Si5351 needs to supply RCLK to the Si4735
 *
 * EXERCISED
 *  + Verifies the Si4735 device appears on the expected I2C bus and reports its address
 *  + PIN_RRST
 *  + PIN_XMT
 *  + PIN_RCV
 *
 * USAGE
 *  pio test -vvv -f "hw/test_04"       // Execute just this test
 *  pio test -vvv -f "hw/*"             // Execute all hw tests
 *
 * NOTES
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "hwdefs.h"
#include <SI4735.h>
#include "patch_full.h"

// Define the I2C parameters for the Si4735
#define I2CBUS WIRE_RCV  // Which Teensy I2C bus hosts the Si4735

// Define a few Si4735 params
#define AM_FUNCTION 1
#define USB 2

// Use the Si4735 library for communication with the chip
SI4735 si4735;

// Si4735 misc params
int16_t si4735Addr;

void setUp(void) {
}

void tearDown(void) {
}

/**
 * @brief Find the Si4735 bus address
 */
void test_si4735_address(void) {
    // Gets and records the Si47XX I2C bus address
    si4735Addr = si4735.getDeviceI2CAddress(PIN_RRST);
    TEST_ASSERT_GREATER_THAN(0, si4735Addr);
    Serial.printf("The Si4735 I2C address is 0x%02x\n", si4735Addr);
}  // test_si4735_address()

/**
 * @brief Exercise Si4735 configuration
 */
uint16_t currentFrequency;
const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h;
void test_config(void) {
    delay(10);
    si4735.queryLibraryId();  // Is it really necessary here? I will check it.
    si4735.patchPowerUp();
    // si4735.setPowerUp(1, 1, 1, 0, 1, SI473X_ANALOG_DIGITAL_AUDIO);
    // si4735.radioPowerUp();
    delay(50);
    int err = si4735.downloadPatch(ssb_patch_content, size_content);
    TEST_ASSERT_NOT_EQUAL(0, err);

    // Parameters
    // AUDIOBW - SSB Audio bandwidth; 0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz;
    // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
    // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
    // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
    // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
    // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
    // si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
    si4735.setSSBConfig(2, 1, 0, 1, 0, 1);  // 2 = 3 kc bandwidth

    delay(10);
    si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
    delay(10);
    si4735.setSSB(18000, 18400, 18100, 1, USB);

    delay(10);
    currentFrequency = si4735.getFrequency();
    si4735.setVolume(50);
    TEST_ASSERT_EQUAL(18100, currentFrequency);
}  // test_config()

/**
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_si4735_address);
    return UNITY_END();
}

void setup() {
    // Initialization
    Serial.begin(9600);  // Test message output device
    I2CBUS.begin();      // I2C bus hosting the Si5351

    // Turn off the transmitter, enable the receiver
    pinMode(PIN_RCV, OUTPUT);
    pinMode(PIN_XMT, OUTPUT);
    digitalWrite(PIN_XMT, LOW);   // XMT off
    digitalWrite(PIN_RCV, HIGH);  // RCV on

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}