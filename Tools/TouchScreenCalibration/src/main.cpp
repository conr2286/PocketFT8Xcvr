/**
 * NAME
 *  TouchScreenCalibration -- Generates calibration coefficients for a resistive touchscreen
 *
 * USAGE
 *  + Execute the TouchScreenCalibration program on the embedded platform
 *  + The program will display touch targets on the screen and await the operator's touch on each.
 *  + Use a stylus for accuracy.
 *  + From time-to-time, the program [re]calculates the calibration coefficients and re-assesses
 *  the touchscreen's accuracy.
 *
 * DEPENDENCIES
 *  + The tool was developed on a Teensy 4.1 MCU using the TeensyDuino (Arduino-Like) framework
 *  with PlatformIO.
 *  + The program was developed for an Adafruit 3.5" TFT 320x480 TouchScreen Breakout with
 *  an HX8357D controller using the Teensy HX8357_t3n driver for the Adafruit_GFX library.
 *
 * NOTES
 *  + This program uses the hardware-defined (i.e. unrotated) touchscreen coordinate system
 *
 */

#include <Arduino.h>
#include <stdint.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include <Fonts/FreeMono12pt7b.h>
// #include "ft8_font.h"
#include "DEBUG.h"
#include "hwdefs.h"

// Display-dependent implementation constants
static const int DISPLAY_WIDTH = 320;   // pixels 0..319
static const int DISPLAY_HEIGHT = 480;  // pixels 0..479

// Touchscreen constants
static const int TS_ADC_COUNT = 1024;  // The number of distinct results from the 10-bit ADC
static const int TS_ACCURACY = 20;     // Pixels (not ADC units)

// Scale factors convert touchpoint ADC readings into uncorrected screen coordinates
static const int TS_X_SCALE = TS_ADC_COUNT / DISPLAY_WIDTH;   // HW x-Axis scale factor
static const int TS_Y_SCALE = TS_ADC_COUNT / DISPLAY_HEIGHT;  // HW y-Axis scale factor

// Define the x and y touchscreen correction values
typedef int8_t TS_CORRECTION_TYPE;                                 // The domain of possible values is -128..+127
static const int TS_CORTAB_COLS = DISPLAY_WIDTH / TS_ACCURACY;     // Number of cells along x-Axis
static const int TS_CORTAB_ROWS = DISPLAY_HEIGHT / TS_ACCURACY;    // Number of cells along y-Axis
static TS_CORRECTION_TYPE tsCorX[TS_CORTAB_COLS][TS_CORTAB_ROWS];  // The X correction values
static TS_CORRECTION_TYPE tsCorY[TS_CORTAB_COLS][TS_CORTAB_ROWS];  // The Y correction values

// Touchpoint coordinates
class TouchPoint {
   public:
    TouchPoint(void) { x = y = z = 0; }

    bool operator==(TouchPoint);
    bool operator!=(TouchPoint);

    int16_t x, y, z;  // Touchpoint screen (*not* ADC) coordinates
};

// Build the Teensy port of the Adafruit graphics library/driver
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

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
    delayMicroseconds(20);                       // Wait for analog Y-Axis signal to settle
    adcZ = analogRead(PIN_YP);                   // Read signal from floating Y-Axis
    floatPin(PIN_XR);                            // Remove Vcc to conserve battery
    touching = isNear(adcZ, 1023, TS_ACCURACY);  // They're touching if Y-Axis ~= VCC

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
        result.y = adcY / TS_Y_SCALE;  // Calc y-Axis screen coordinate
        result.x = adcX / TS_X_SCALE;  // Calc x-Axis screen coordinate
        result.z = adcZ;               // We never calculate the so-called touch pressure
    } else {
        result.x = result.y = result.z = 0;  // Operator is not touching the pad
    }

    return result;
}  // getPoint()

/**
 * @brief Get the corrected TouchPoint value
 * @param rawP The uncorrected TouchPoint in the unrotated screen coordinate system
 * @return The corrected TouchPoint
 *
 * @note This function corrects for the margins and non-linearity across an axis.
 */
TouchPoint getCorrectedTouchPoint(TouchPoint rawP) {
    TouchPoint result;
    result.z = rawP.z;  // Return z unmodified
    if (rawP.z) {
        // DTRACE();
        int nRow = rawP.y / TS_CORTAB_ROWS;  // Select a row in the correction table
        int nCol = rawP.x / TS_CORTAB_COLS;  // Select a column in the correction table
        DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
        DPRINTF("using tsCorX = %d tsCorY = %d\n", tsCorX[nCol][nRow], tsCorY[nCol][nRow]);
        result.x = rawP.x + tsCorX[nCol][nRow];  // Correct the x coordinate
        result.y = rawP.y + tsCorY[nCol][nRow];  // Correct the y coordinate
        DPRINTF("rawP.x=%d rawP.y=%d nRow=%d nCol=%d result.x=%d result.y=%d\n", rawP.x, rawP.y, nRow, nCol, result.x, result.y);
    }
    return result;
}  // getCorrectedTouchPoint()

int updateCorTab(TouchPoint expectedP, TouchPoint actualP) {
    // Calculate the errors on both axis
    int errX = actualP.x - expectedP.x;  // Error on x-Axis
    int errY = actualP.y - expectedP.y;  // Error on y-Axis

    // Update correction table from errors
    int nRow = actualP.y / TS_CORTAB_ROWS;  // Select a row in the correction table
    int nCol = actualP.x / TS_CORTAB_COLS;  // Select a column in the correction table

    DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
    tsCorX[nCol][nRow] -= errX;  // Update the x-Axis correction
    tsCorY[nCol][nRow] -= errY;  // Update the y-Axis correction
    DPRINTF("updated tsCorX = %d tsCorY = %d\n", tsCorX[nCol][nRow], tsCorY[nCol][nRow]);

    // Return the root mean square of the x/y error
    int errRMS = sqrt(errX * errX + errY * errY);  // Root mean square of errors
    DPRINTF("expectedP.x=%d actualP.x=%d errX=%d   expectedP.y=%d actualP.y=%d errY=%d   errRMS=%d\n", expectedP.x, actualP.x, errX, expectedP.y, actualP.y, errY, errRMS);
    DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
    return errRMS;
}

/**
 * @brief Exercises a single target on the touchscreen
 * @param x Target's x-Coord
 * @param y Target's y-Coord
 * @return Touchpoint deviation (in pixels) from target coordinates
 */
unsigned exerciseTouchTarget(int x, int y) {
    TouchPoint rawP;  // Raw display system coordinates of touchpoint
    TouchPoint corP;  // Corrected display system coordinates of touchpoint

    DPRINTF("\n");

    // Erase screen and display operator prompt and touch target
    tft.fillScreen(HX8357_BLACK);                          // Erase screen to black
    tft.setCursor(x, y);                                   // Display the target
    tft.fillCircle(x, y, TS_ACCURACY / 2, HX8357_YELLOW);  // Draw the target

    // Wait for the operator to touch the target
    while (rawP.z == 0) {
        rawP = getTouchPoint();
        if (rawP.z == 0) delay(10);
    }
    DPRINTF("rawP.x=%d rawP.y=%d\n", rawP.x, rawP.y);

    // Apply existing correction and calculate error
    corP = getCorrectedTouchPoint(rawP);  // Apply the correction factor

    // Update correction tables from current error
    TouchPoint expectedP;
    expectedP.x = x;
    expectedP.y = y;
    expectedP.z = 1;
    int errRMS = updateCorTab(expectedP, corP);

    // Return the RMS error
    return errRMS;
}

/**
 * @brief Program initialization
 */
void setup() {
    // Get the USB Serial port working for debugging
    Serial.begin(9600);
    DPRINTF("Starting...\n");

    // Initialize the touchscreen correction table
    for (int x = 0; x < TS_CORTAB_COLS; x++) {
        for (int y = 0; y < TS_CORTAB_ROWS; y++) {
            tsCorX[x][y] = 0;  // No x correction required
            tsCorY[x][y] = 0;  // No y correction required
        }
    }

    // Initialize the display
    tft.begin(30000000UL, 2000000UL);
    tft.setFont(&FreeMono12pt7b);
    tft.fillScreen(HX8357_BLACK);  // Erase screen

    int err;
    err = exerciseTouchTarget(310, 10);
    DPRINTF("err=%d\n", err);
    delay(500);
    err = exerciseTouchTarget(310, 10);
    DPRINTF("err=%d\n", err);
}

/**
 * @brief The great polling loop
 */
void loop() {
    // Display prompt, target and read touchpoint coordinates
}
