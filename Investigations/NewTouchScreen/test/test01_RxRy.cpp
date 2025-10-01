/**
 * @brief test01_RxRy --- Calculate the resistive touchscreen Rx and Ry values
 *
 * This basic test calculates the values of the touchscreen's X-Resistance (Rx)
 * and Y-Resistance (Ry) and verifies they are reasonable.  On Pocket FT8 V2.00 and
 * V3.00 boards, the touchscreen is wired as follows:
 *
 *      PIN_XM -----/\/\/\-------+-------/\/\/\-----+-----/\/\/\----- PIN_XP
 *                    510        |         Rx1      |       Rx2
 *                              AD2                 /
 *                                                  \
 *                                                  /  Rz
 *                                                  \
 *                                                  /
 *                              AD1                 \
 *                    510        |         Ry1      |       Ry2
 *      PIN_YP -----/\/\/\-------+-------/\/\/\-----+-----/\/\/\----- PIN_YM
 *
 *  Where:  Rx = Rx1+Rx2, Ry = Ry1+Ry2 --- The X and Y plane total resistances
 *          Rz is the variable touch pressure resistance
 *          AD1 and AD2 are the two MCP342x ADC inputs
 *          PIN_XM, PIN_XP, PIN_YP and PIN_YM are Teensy 4.2 GPIO pins
 *
 *  Notes:  Without a touch, Rz should be near infinite.  During a touch event, Rz becomes
 *          proportional to the touch pressure and connects the X-Plane with the Y-Plane
 *          as implied in the illustration above.  While the exact values of Rx1 and Rx2
 *          vary during a touch event, their sum always totals Rx, the total resistance
 *          of the X-Plane.  The Y-Plane behaves similarly.
 *
 *          Each ADC channel has 12 bits of resolution with a maximum reading of 2047
 *          referenced internally to correspond to 2.047 Volts.  The 510 Ohm resistors'
 *          values are chosen such that any combination of GPIO signals from PIN_XM,
 *          PIN_XP, PIN_YP or PIN_YM will never exceed 2.047 Volts with any expected
 *          value of Rx or Ry.
 *
 *          None of these tests require the operator to actually touch the screen.
 *
 * To Measure Rx:  Drive PIN_XM to Vcc, ground PIN_XP, float PIN_YP and YM, and read AD2.
 *
 * To Measure Ry:  Drive PIN_YP to Vcc, ground PIN_YM, float PIN_XM and XP, and read AD1.
 */
#include <Arduino.h>
#include <unity.h>

#include "pins.h"
#include <Wire.h>
#include "MCP342x.h"

// Build the ADC object for accessing the resistive touchpad
#define WIRE WIRE_ETC            // Define which I2C bus hosts the ADC
uint8_t address = 0x69;          // Original I2C address was 0x68 but V1.01 chip reports 0x69
MCP342x adc = MCP342x(address);  // TheADC object

// The tests calculate Rx and Ry, the total resistance of the X and Y planes
float Rx, Ry;     // The X/Y plane resistances
float Vx, Vy;     // The bias applied to the X/Y planes thru their 510 Ohm resistors
float Vcc = 3.3;  // Teensy Vcc

// Function invoked by Unity prior to executing each test case
void setUp(void) {
}

// Function invooked by Unity following each test case
void tearDown(void) {
}

// Verify ADCs read ~0 Volts with all pins grounded
void test_grounds(void) {
    // Configure all four GPIO pins for output
    pinMode(PIN_YP, OUTPUT);
    pinMode(PIN_XP, OUTPUT);
    pinMode(PIN_YM, OUTPUT);
    pinMode(PIN_XM, OUTPUT);

    // Ground all four GPIO pins
    digitalWrite(PIN_YP, LOW);
    digitalWrite(PIN_XP, LOW);
    digitalWrite(PIN_YM, LOW);
    digitalWrite(PIN_XM, LOW);
    delayMicroseconds(20);  // Fast ARM chips need to allow voltages to settle

    // Expect ~0 Volts into each ADC
    long xValue = 0, yValue = 0;
    MCP342x::error_t err;
    MCP342x::Config status;
    err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 1000000, xValue, status);
    TEST_ASSERT_EQUAL_INT32(MCP342x::errorNone, err);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(1, xValue);
    err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 1000000, yValue, status);
    TEST_ASSERT_EQUAL_INT32(MCP342x::errorNone, err);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(1, yValue);
}

// Verify ADCs read ~2.4 Volts with PIN_XM=PIN_YP=Vcc while PIN_XP and PIN_YM held at ground
void test_highV(void) {
    // Configure all four GPIO pins for output
    pinMode(PIN_YP, OUTPUT);
    pinMode(PIN_XP, OUTPUT);
    pinMode(PIN_YM, OUTPUT);
    pinMode(PIN_XM, OUTPUT);

    // Hold PIN_XM=PIN_YP=Vcc while PIN_XP and PIN_YM held at ground
    digitalWrite(PIN_XM, HIGH);
    digitalWrite(PIN_YP, HIGH);
    digitalWrite(PIN_XP, LOW);
    digitalWrite(PIN_YM, LOW);
    delayMicroseconds(20);  // Fast ARM chips need to allow voltages to settle

    // Expect 2 Volts into each ADC
    long xValue = 0, yValue = 0;
    MCP342x::error_t err;
    MCP342x::Config status;
    err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 1000000, xValue, status);
    TEST_ASSERT_EQUAL_INT32(MCP342x::errorNone, err);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(2047, xValue);
    TEST_ASSERT_GREATER_OR_EQUAL_INT32(1000, xValue);
    err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 1000000, yValue, status);
    TEST_ASSERT_EQUAL_INT32(MCP342x::errorNone, err);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(2047, yValue);
    TEST_ASSERT_GREATER_OR_EQUAL_INT32(1000, yValue);

    // Calculate Vx and Vy, the bias voltages applied to the X and Y planes thru their 510 Ohm resistors
    Vx = xValue / 1000.0;
    Vy = yValue / 1000.0;
    Serial.printf("Vx=%f, Vy=%f\n", Vx, Vy);

    // Calculate Rx and Ry, the resistance of the untouched X and Y planes
    Rx = (510.0 * Vx) / (Vcc - Vx);
    Ry = (510.0 * Vy) / (Vcc - Vy);
    Serial.printf("Rx=%f, Ry=%f\n", Rx, Ry);
}

// Run all Unity test cases in this suite
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_grounds);
    RUN_TEST(test_highV);
    return UNITY_END();
}

void setup() {
    Serial.begin(9600);

    WIRE.begin();
    MCP342x::generalCallReset();
    delay(1);  // MC342x needs 300us to settle, wait 1ms

    // Run the tests
    runUnityTests();
}

void loop() {
    // delay(1000);
}