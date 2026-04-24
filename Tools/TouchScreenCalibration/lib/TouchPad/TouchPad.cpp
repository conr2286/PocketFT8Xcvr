/**
 * @brief TouchPad Driver
 *
 * This library implements a C "driver" for the resistive Adafruit 3.5" TFT 320x480
 * Touchscreen Breakout Board w/MicroSD Socket - HX8357D (Product 2050).
 *
 * Sadly, there are a couple weirdo artifacts (the X and Y axis 510 Ohm resistors,
 * XR and YR) specific to the PocketFT8Xcvr hardware and Teensy debugging support.
 */

#include <Arduino.h>
#include <stdint.h>

#include "DEBUG.h"
// #include "hwdefs.h"
#include "TouchPad.h"

// Define the number of microseconds to allow analog signals to settle prior to ADC sampling
static const unsigned ADC_SETTLE_US = 20;

/**
 * @brief Construct a TouchPad object
 * @param xp MCU pin for touchpad's X+ signal
 * @param xm MCU analog pin for touchpad's X- signal
 * @param yp MCU analog pin for touchpad's Y+ signal
 * @param ym MCU pin for touchpad's Y- signal
 * @param xr The x-Axis 510 Ohm resistor
 * @param yr The y-Axis 510 Ohm resistor
 *
 * @note Note where MCU analog pins are required as their use here is a bit
 * unconventional to deal with peculiarities of the Adafruit touchpad HW.
 *
 * @note Perhaps this should be a singleton but if you really need more than
 * one touchpad then I guess you can have them.
 *
 * @note The PocketFT8Xcvr hardware has a 510 ohm resistor connected to each
 * axis... a bit unusual and left over from legacy hardware.  We use them
 * here to momentarily ground an otherwise floating axis to discharge
 * accumulated noise.  I'm not sure if this remains necessary on current HW
 * but, because they remain on the board, we at least have to float them to
 * avoid interfering with the ADC readings.
 *
 */
TouchPad::TouchPad(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t xr, uint8_t yr) {
    this->xp = xp;
    this->xm = xm;
    this->yp = yp;
    this->ym = ym;
    this->xr = xr;
    this->yr = yr;
    this->state = TS_NO_TOUCH;  // Assume nothing is touching the pad

    // TODO:  Initialize these in a more portable fashion as these are bound to Adafruit T3.5" TFT and Teensy ADC
    this->adcMax = 1023;  // Maximum value returned by ADC
    this->nCols = 320;    // Number of touchscreen columns
    this->nRows = 480;    // Number of touchscreen rows
}

/**
 * @brief Determine if a is approximately equal to b
 * @param a value1
 * @param b value2
 * @param delta allowed difference
 * @return true if a lies within the allowed difference of b
 */
bool TouchPad::isNear(unsigned a, unsigned b, unsigned delta) {
    if ((a >= b - delta) && (a <= b + delta)) return true;
    return false;
}  // isNear()

/**
 * @brief Helper function to configure GPIO pin to float
 * @param pin GPIO pin number
 *
 * @note Floating a pin removes the digital I/O circuitry from that pin
 */
void TouchPad::floatPin(unsigned pin) {
    pinMode(pin, INPUT_DISABLE);  // Floats a Teensy 4.1 GPIO pin
}  // floatPin()

/**
 * @brief Helper function to "ground" a pin
 * @param pin GPIO pin number
 *
 * @note The specified pin is brought to logic level zero, close to but not
 * truly ground.
 */
void TouchPad::groundPin(unsigned pin) {
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
void TouchPad::vccPin(unsigned pin) {
    pinMode(pin, OUTPUT);     // Configure this pin for digital output
    digitalWrite(pin, HIGH);  // Supply logic one to pin
}  // vccPin()

/**
 * @brief Non-blocking read of touchscreen coordinates
 * @return TouchPoint coordinates as raw 10-bit ADC readings
 *
 * NOTES:
 *  + Returned TouchPoint.z=TS_NO_TOUCH if the x/y result is invalid
 *  + Returned TouchPoint x/y are in raw, unfiltered, unscaled ADC values
 *  + The x value corresponds to the HW x-Axis as does the y value to the HW y-Axis
 *  + Does not discern dragging
 *
 * DESIGN:
 *  + The conventional approach, calculating z (estimated touch pressure) is not
 *  used here to identify a valid touch event.  Our simplified approach drives the
 *  entire x-Axis to Vcc and then measures the signal on the floating y-Axis,
 *  expecting to see ~Vcc on the y-Axis during a valid touch event.
 *  + Resistive touchpads are notoriously nonlinear but no correction/calibration
 *  is performed here.  We just acquire the ADC readings.
 *  + The touchpad signals are driven through the MCU digital outputs, and are read
 *  using the MCU ADC and Arduino analogRead() which, on the Teensy 4.1 MCU, will be
 *  using ADC2 by default as ADC1 is assigned to the Teensy audio pipeline.
 *  + Use getTouchEvent to distinguish between TS_TOUCH and TS_DRAG
 */
TouchPoint TouchPad::getTouchPoint(void) {
    TouchPoint result;
    int adcX, adcY, adcZ;  // The unscaled ADC readings
    bool touching;         // true ==> valid touch event

    // Setup to determine if we have a valid touch event (i.e. operator
    // is actually touching the pad) by driving the X-Axis to Vcc and
    // measuring the y-Axis signal.  During a valid touch event, the X
    // and Y axis connect through operator's pressure on the pad,
    // placing Vcc on the normally floating y-Axis.
    floatPin(xp);  // Float X+
    floatPin(xm);  // Also X-
    vccPin(xr);    // Drive the X-Axis to Vcc through its 510 Ohm resistor

    // Clear the noise from High-Z Y-Axis prior to the analog reading below
    floatPin(ym);                      // Float Y-
    groundPin(yr);                     // Discharge Y-Axis thru its 510 Ohm resistor to ground
    delayMicroseconds(ADC_SETTLE_US);  // Wait for analog Y-Axis signal to settle to ground
    floatPin(yr);                      // Remove discharge path
    floatPin(yp);                      // And disconnect all digital circuitry from the analog signal

    // Vcc flows through the driven X-Axis connection to floating Y-Axis iff we have a touch event
    delayMicroseconds(ADC_SETTLE_US);   // Wait for analog Y-Axis signal to settle
    adcZ = analogRead(yp);              // Read signal from floating Y-Axis
    floatPin(xr);                       // Remove Vcc to conserve battery
    touching = isNear(adcZ, 1023, 20);  // They're touching if Y-Axis ~= VCC

    // If we have a valid touchpoint then proceed to read the X and Y-Axis coordinates.  Because
    // the X and Y-Axis connect thru touch pressure, we have a relatively noise free low-Z signal.
    if (touching) {
        // DTRACE();
        //  Setup touchpad to read the hardware X-Axis signal from the Y-Axis
        floatPin(ym);   // Disconnect digital I/O circuitry from...
        floatPin(yr);   // all the Y-Axis pins.
        floatPin(yp);   //
        groundPin(xp);  // Ground touchpad X+
        floatPin(xr);   // We are not using the 510 Ohm resistor
        vccPin(xm);     // Drive XM to Vcc, leaving yp sampling the voltage divider

        // Read the touchpoint hardware X-Coordinate signal from the Y-Axis
        delayMicroseconds(ADC_SETTLE_US);  // Allow analog signal to settle
        adcX = analogRead(yp);             // Read X-Coord from the floating Y-Axis
        floatPin(xm);                      // Remove Vcc to conserve battery

        // Setup touchpad to read the hardware Y-Axis signal from the floating X-Axis
        // Note:  For the touchpad coords to match default GFX coords, we have to
        // reverse the drive on the Y-Axis as they don't agree on location of origin.
        floatPin(xp);   // Disconnect digital I/O circuitry...
        floatPin(xr);   // from all the X-Axis pins.
        floatPin(xm);   //
        floatPin(yr);   // Float Y-Axis 510 Ohm resistor
        groundPin(yp);  // Ground YP (for the reverse drive)
        vccPin(ym);     // Drive YM to Vcc (for the reverse drive)

        // Read the touchpoint's Y-Axis coordinate signal from the floating X-Axis
        delayMicroseconds(ADC_SETTLE_US);  // Allow analog signals to settle
        adcY = analogRead(xm);             // Raw ADC value ranges 0..1023
        floatPin(yp);                      // Remove Vcc to save battery

        // Return a valid result in the unrotated, uncorrected screen coordinate system
        result.y = adcY;      // Calc y-Axis screen coordinate
        result.x = adcX;      // Calc x-Axis screen coordinate
        result.z = TS_TOUCH;  //
        // DPRINTF("result.x=%d result.y=%d result.z=%d\n", result.x, result.y, result.z);
    } else {
        result.x = result.y = 0;
        result.z = TS_NO_TOUCH;  // Operator is not touching the pad
    }

    return result;
}  // getTouchPoint()

template <int N>
static void sort_small(uint16_t (&a)[N]) {
    for (int i = 1; i < N; i++) {
        uint16_t v = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > v) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = v;
    }
}

/**
 * @brief Filtered, stateful interrogation of touchpad
 * @return TouchPoint
 *
 * DISCUSSION:
 *  + We currently average two readings.  This may change if problematic.
 *  + We maintain a state variable to discern when a touch event begins, when
 *  the stylus drags across the pad, and when the stylus has been removed.
 *  + Coordinates are returned as raw ADC values, not screen coordinates.
 *
 */
TouchPoint TouchPad::getTouchEvent(void) {
    TouchPoint result;  // We depend upon constructor initializing their attributes

    const int N = 7;
    uint16_t xs[N], ys[N];

    for (int i = 0; i < N; i++) {
        TouchPoint raw = getTouchPoint();

        // Handle invalid (not touching, erroneous, whatever) readings
        if (raw.z == TS_NO_TOUCH) {
            state = result.z = TS_NO_TOUCH;  // Record touch event state as not touching and inform caller
            return result;                   // TS_NO_TOUCH
        }

        // Save readings
        xs[i] = raw.x;
        ys[i] = raw.y;
        // DPRINTF("%d: raw.x=%d raw.y=%d raw.z=%d\n", i, raw.x, raw.y, raw.z);
    }  // for

    // Filter
    sort_small(xs);
    sort_small(ys);
    result.x = xs[N / 2];  // Median x value
    result.y = ys[N / 2];  // Median y value

    // Analyze state
    switch (state) {
        // We have begun a new touch event
        case TS_NO_TOUCH:
            state = result.z = TS_TOUCH;  // Newly touched
            break;

        // We are dragging
        case TS_TOUCH:
        case TS_DRAG:
            state = result.z = TS_DRAG;  // Begun dragging
            break;

        default:
            break;
    }

    // DPRINTF("state=%d result.x=%d result.y=%d result.z=%d\n", state, result.x, result.y, result.z);

    return result;
}
