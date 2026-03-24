/**
 * TEST
 *  + This test confirms connectivity with the Si4735
 *
 * PREREQUISITES
 *  + The Teensy MCU needs to be able to successfully load and execute a test (e.g. test_blinky)
 *  + The Si5351 needs to supply RCLK to the Si4735 when in receive mode
 *
 * EXERCISED
 *  + Verifies the Si4735 device appears on the expected I2C bus and reports its address
 *  + PIN_RRST
 *  + PIN_XMT
 *  + PIN_RCV
 *  + Loads SSB patch into Si4735
 *  + Confirms configured frequency
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
#include <si5351.h>
#include <SI4735.h>

#include "patch_full.h"
#include "hwdefs.h"

// Define the I2C parameters for the Si4735
#define I2CBUS WIRE_RCV  // Which Teensy I2C bus hosts the Si4735

// Define a few Si4735 params
#define AM_FUNCTION 1
#define USB 2

// The Si4735 PLL locks to the Si5351 RCLK signal
Si5351 si5351;  // We need the Si5351 clock running

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
}  // test_config()

void test_params(void) {
    delay(100);
    si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
    delay(10);
    si4735.setSSB(7000, 7300, 7074, 1, USB);

    delay(1000);

    si4735.setVolume(50);
    TEST_ASSERT_EQUAL(50, si4735.getVolume());

    si4735.setFrequency(7074);
    TEST_ASSERT_EQUAL(7074, si4735.getFrequency());

}  // test_config()

/**
 *
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_si4735_address);
    RUN_TEST(test_config);
    RUN_TEST(test_params);
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

    // Initialize the SI5351 clock generator (the Si4735's PLL requires the Si5351 RCLK signal).
    // NOTE:  PocketFT8Xcvr boards use CLKIN input (supposedly less jitter than the XTAL pins).
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000L, 0L);  // KQ7B's counter isn't accurate enough to calculate a correction
    delay(10);
    si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);  // We are using cmos CLKIN, not a XTAL input!!!
    delay(10);
    // si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);  // All PLLs using CLKIN
    // delay(10);
    // si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);              // Fixed point division offers less jitter
    // delay(10);
    si5351.set_freq(3276800, SI5351_CLK2);  // Receiver's fixed frequency clock for Si4735 PLL
    delay(10);
    si5351.output_enable(SI5351_CLK2, 1);  // Receiver's clock is always on
    delay(10);
    si5351.output_enable(SI5351_CLK0, 0);  // Disable transmitter clock for this test
    delay(10);

    // Run the tests
    runUnityTests();

    delay(10000);
}

void loop() {
    // delay(1000);
}