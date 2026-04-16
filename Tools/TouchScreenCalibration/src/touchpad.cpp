#include <Arduino.h>
#include <stdint.h>

#include "DEBUG.h"
#include "hwdefs.h"
#include "touchpad.h"

/**
 * @brief Determine if a is approximately equal to b
 * @param a value1
 * @param b value2
 * @param delta allowed difference
 * @return true if a lies within the allowed difference of b
 */
bool isNear(unsigned a, unsigned b, unsigned delta) {
    if ((a >= b - delta) && (a <= b + delta)) return true;
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
    pinMode(pin, OUTPUT);     // Configure pin for digital output
    digitalWrite(pin, HIGH);  // Supply logic one to pin
}  // vccPin()

/**
 * @brief Non-blocking read of touchscreen coordinates
 * @return TouchPoint coordinates in the uncorrected screen coordinate system
 *
 * NOTES:
 *  + The returned TouchPoint.z=0 if the x/y result is invalid
 *  + The returned TouchPoint.z=ADC(Vcc) if the x/y result appears valid
 *  + The returned TouchPoint.z>0 if the x/y values are valid coordinates
 *  + The x and y values are unrotated... x/y correspond to the HW x and y axis
 *
 * DESIGN:
 *  + The conventional approach, calculating z (the touch pressure) is not used
 *  to identify a valid touch event.  Our simplified approach drives the
 *  entire x-Axis to Vcc and measures the signal on the floating y-Axis,
 *  expecting to see ~Vcc on the y-Axis during a valid touch event.
 */
TouchPoint getTouchPoint(void) {
    TouchPoint result;
    int adcX, adcY, adcZ;  // The unscaled ADC readings
    bool touching;         // true ==> valid touch event

    // Setup to determine if we have a valid touch event (i.e. operator
    // is actually touching the pad) by driving the X-Axis to Vcc and
    // measuring the y-Axis signal.  During a valid touch event, the X
    // and Y axis are connected through operator's pressure on the pad.
    floatPin(PIN_XP);  // Float X+
    floatPin(PIN_XM);  // Also X-
    vccPin(PIN_XR);    // Drive the X-Axis to Vcc through its 510 Ohm resistor

    // Now clear the noise from High-Z Y-Axis prior to the analog reading below
    floatPin(PIN_YM);       // Float Y-
    groundPin(PIN_YR);      // Discharge Y-Axis thru its 510 Ohm resistor to ground
    delayMicroseconds(20);  // Wait for analog Y-Axis signal to settle to ground
    floatPin(PIN_YR);       // Remove discharge path
    floatPin(PIN_YP);       // And disconnect all digital circuitry from the analog signal

    // Vcc flows through the driven X-Axis connection to floating Y-Axis iff we have a touch event
    delayMicroseconds(20);              // Wait for analog Y-Axis signal to settle
    adcZ = analogRead(PIN_YP);          // Read signal from floating Y-Axis
    floatPin(PIN_XR);                   // Remove Vcc to conserve battery
    touching = isNear(adcZ, 1023, 20);  // They're touching if Y-Axis ~= VCC

    // If we have a valid touchpoint then proceed to read the X and Y-Axis coordinates.  Because
    // the X and Y-Axis connect thru touch pressure, we have a relatively noise free low-Z signal.
    if (touching) {
        // Setup touchpad to read the hardware X-Axis signal from the Y-Axis
        floatPin(PIN_YM);   // Disconnect digital I/O circuitry from...
        floatPin(PIN_YR);   // all the Y-Axis pins.
        floatPin(PIN_YP);   //
        groundPin(PIN_XP);  // Ground touchpad X+
        floatPin(PIN_XR);   // We are not using the 510 Ohm resistor
        vccPin(PIN_XM);     // Drive XM to Vcc, leaving PIN_YP sampling the voltage divider

        // Read the touchpoint hardware X-Coordinate signal from the Y-Axis
        delayMicroseconds(20);      // Allow analog signal to settle
        adcX = analogRead(PIN_YP);  // Read X-Coord from the floating Y-Axis
        floatPin(PIN_XM);           // Remove Vcc to conserve battery

        // Setup touchpad to read the hardware Y-Axis signal from the floating X-Axis
        // Note:  For the touchpad coords to match default GFX coords, we have to
        // reverse the drive on the Y-Axis as they don't agree on location of origin.
        floatPin(PIN_XP);   // Disconnect digital I/O circuitry...
        floatPin(PIN_XR);   // from all the X-Axis pins.
        floatPin(PIN_XM);   //
        floatPin(PIN_YR);   // Float Y-Axis 510 Ohm resistor
        groundPin(PIN_YP);  // Ground YP (for the reverse drive)
        vccPin(PIN_YM);     // Drive YM to Vcc (for the reverse drive)

        // Read the touchpoint's Y-Axis coordinate signal from the floating X-Axis
        delayMicroseconds(20);      // Allow analog signals to settle
        adcY = analogRead(PIN_XM);  // Raw ADC value ranges 0..1023
        floatPin(PIN_YP);           // Remove Vcc to save battery

        // Return a valid result in the unrotated, uncorrected screen coordinate system
        result.y = adcY;  // Calc y-Axis screen coordinate
        result.x = adcX;  // Calc x-Axis screen coordinate
        result.z = adcZ;  // This is actually Vcc (logic 1)
    } else {
        result.x = result.y = result.z = 0;  // Operator is not touching the pad
    }

    return result;
}  // getPoint()
