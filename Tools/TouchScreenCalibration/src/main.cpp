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
 */

#include <Arduino.h>
#include <stdint.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include <Fonts/FreeMono12pt7b.h>
// #include "ft8_font.h"
#include "DEBUG.h"
#include "hwdefs.h"
#include "TouchPad.h"

// Display-dependent implementation constants
static const int DISPLAY_WIDTH = 320;                         // pixels 0..319
static const int DISPLAY_WIDTH_EXTENT = DISPLAY_WIDTH - 1;    // Rightmost column
static const int DISPLAY_HEIGHT = 480;                        // pixels 0..479
static const int DISPLAY_HEIGHT_EXTENT = DISPLAY_HEIGHT - 1;  // Bottom row

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

// Build the Teensy port of the Adafruit graphics library/driver
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

// Build the Adafruit TFT 320x480 touchpad driver
TouchPad touchPad = TouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

// Define a TargetPoint tuple containing the target and actual coordinates from touch event
class TargetPoint {
   public:
    TouchPoint target;  // Coordinates of the target (on the screen)
    TouchPoint actual;  // Coordinates from touch event (from the HID)
};

/**
 * @brief Read coordinates of the next touch event
 * @return TouchPoint coordinates in the screen coordinate system
 *
 * @note This is a blocking read of the next touch event
 *
 * @note Touchpads are notoriously noisy.  We mitigate this problem by taking
 * multiple ADC readings.
 *
 * TODO:  If we had an isTouching state variable, we could optimize by only
 * waiting for operator to pick-up the stylus prior to the next read
 */
TouchPoint getNextTouchPoint(void) {
    TouchPoint raw1, raw2, result;
    bool unstable = true;

    DTRACE();

    while (unstable) {
        // Acquire two readings from the touchpad
        raw1 = touchPad.getTouchPoint();
        raw2 = touchPad.getTouchPoint();

        // Are the readings valid and stable?
        if ((raw1.z && raw2.z) && touchPad.isNear(raw1.x, raw2.x, TS_ACCURACY) && touchPad.isNear(raw1.y, raw2.y, TS_ACCURACY)) break;  // Yes, we have valid data

        // Wait before trying again
        delay(50);
        // DTRACE();
    }

    // Map averaged ADC coordinates into the screen coordinate system
    result.x = map((raw1.x + raw2.x) / 2, 0, 1023, 0, DISPLAY_WIDTH_EXTENT);
    result.y = map((raw1.y + raw2.y) / 2, 0, 1023, 0, DISPLAY_HEIGHT_EXTENT);
    result.z = 1;
    DPRINTF("result.x=%d result.y=%d\n", result.x, result.y);

    // Wait for operator to remove the stylus from pad
    // while (touchPad.getTouchPoint().z) {
    //     delay(10);  // Wait a moment
    // }
    // delay(50);

    return result;

}  // getNextTouchPoint()

/**
 * @brief Displays a prompt message for operator on the TFT
 * @param x Screen coordinate where target will appear
 * @param y Screen coordinate where target will appear
 */
void promptOperator(unsigned x, unsigned y) {
    DTRACE();
    tft.fillScreen(HX8357_BLACK);  // Erase screen
    tft.setCursor(20, 20);
    tft.setTextColor(HX8357_WHITE);
    tft.printf("Touch target at (%d,%d)\n", x, y);
}  // promptOperator

/**
 * @brief Display touch target on the TFT
 * @param x Screen coordinate for target
 * @param y Screen coordinate for target
 */
void displayTarget(unsigned x, unsigned y) {
    DTRACE();
    tft.fillCircle(x, y, 5, HX8357_YELLOW);
}  // displayTarget()

/**
 * @brief Prompt operator, display target, and read touchpad result
 * @param x Target's screen coordinate
 * @param y Target's screen coordinate
 * @return Touchpoint coordinates
 */
TargetPoint getTargetPoint(unsigned x, unsigned y) {
    TargetPoint p;
    DTRACE();
    p.target.x = x;
    p.target.y = y;
    p.target.z = 1;
    promptOperator(x, y);            // Inform operator what's happening
    displayTarget(x, y);             // Show operator where to touch
    p.actual = getNextTouchPoint();  // Read next touchpad coordinates
    return p;
}  // getTargetPoint()

/**
 * @brief Initializes the interpolator
 *
 * @note Should be invoked only once
 *
 * Initializes the interpolator's correction matrices to 0 (meaning, no adjustment required)
 */
void initInterpolator(void) {
    for (int col = 0; col < TS_CORTAB_COLS; col++) {
        for (int row = 0; row < TS_CORTAB_ROWS; row++) {
            tsCorX[col][row] = 0;
            tsCorY[col][row] = 0;
        }
    }
}  // initInterpolator()

/**
 * @brief Builds the interpolation matrix within the specified rectangle
 * @param ul Upper-left tuple containing target and actual touch event coordinates in screen system
 * @param ur Upper-right tuple containing target and actual touch event coordinates in screen system
 * @param lr Lower-left tuple containing target and actual touch event coordinates in screen system
 * @param ll Lower-right tuple containing target and actual touch event coordinates in screen system
 */
void buildInterpolationMatrix(TargetPoint ul, TargetPoint ur, TargetPoint lr, TargetPoint ll) {
    // TODO
}  // buildInterpolationMatrix()

/**
 * @brief Use interpolation matrix to map touchpad coordinates to the screen
 * @param p Touchpad reading (in the screen coordinate system)
 * @return Corrected coordinate (also in screen coordinate system)
 *
 * @note Returns z=0 if result is invalid
 */
TouchPoint getCorrectedPoint(TouchPoint p) {
    TouchPoint c;

    DTRACE();

#if 1
    // Select a row and column in the correction matrices
    unsigned corRow = p.y / TS_CORTAB_ROWS;
    unsigned corCol = p.x / TS_CORTAB_COLS;
    DPRINTF("corCol=%d corRow=%d p.x=%d p.y=%d p.z=%d\n", corCol, corRow, p.x, p.y, p.z);

    // Sanity checks
    if ((corRow >= TS_CORTAB_ROWS) || (corCol >= TS_CORTAB_COLS) || (p.z == 0)) {
        DTRACE();
        c.x = c.y = c.z = 0;  // Return error indication
        return c;
    }

    // Apply correction matrices to uncorrected coordinate
    // c.x = p.x + tsCorX[p.x][p.y];  // x-Axis correction from tsCorX column x, row y
    // c.y = p.y + tsCorY[p.x][p.y];  // y-Axis correction from tsCorY column x, row y
    c.x = p.x + tsCorX[corCol][corRow];  // x-Axis correction from tsCorX column x, row y
    c.y = p.y + tsCorY[corCol][corRow];  // y-Axis correction from tsCorY column x, row y
#else
    c.x = p.x;
    c.y = p.y;
#endif

    c.z = p.z;
    DPRINTF("c.x=%d c.y=%d c.z=%d\n", c.x, c.y, c.z);
    return c;
}  // getCorrectedPoint()

/**
 * @brief Program initialization
 */
void setup() {
    // Get the USB Serial port working for debugging
    Serial.begin(9600);
    DPRINTF("Starting...\n");

    // Initialize the display
    tft.begin(30000000UL, 2000000UL);
    tft.setFont(&FreeMono12pt7b);  // Use a readable size, standard font

    // Initialize the interpolator
    initInterpolator();

    // Display targets in corners and get their uncorrected coordinates
    TargetPoint p1 = getTargetPoint(0, 0);                                         // Find the upper-left corner
    TargetPoint p2 = getTargetPoint(DISPLAY_WIDTH_EXTENT, 000);                    // Find the upper-right corner
    TargetPoint p3 = getTargetPoint(DISPLAY_WIDTH_EXTENT, DISPLAY_HEIGHT_EXTENT);  // Find the lower-right corner
    TargetPoint p4 = getTargetPoint(0, DISPLAY_HEIGHT_EXTENT);                     // Find lower-left corner

    // Interpolate the quadrilateral bounded by (p1,p2) and (p3,p4)
    buildInterpolationMatrix(p1, p2, p3, p4);

    // Erase the display and prompt operator to drag stylus around the screen
    tft.fillScreen(HX8357_BLACK);
    tft.setCursor(20, 20);
    tft.setTextColor(HX8357_WHITE);
    tft.printf("Drag stylus around touchpad");
}  // setup()

/**
 * @brief The great Arduino polling loop tracks the stylus on touchscreen
 */
void loop() {
    TouchPoint p1 = getNextTouchPoint();                         // Read the touchpad
    if (p1.z) {                                                  // Check for valid touch event
        TouchPoint p2 = getCorrectedPoint(p1);                   // Map p1 to p2 using interpolation matrices
        if (p2.z) tft.fillCircle(p1.x, p1.y, 3, HX8357_YELLOW);  // Show operator where we think the stylus touched the pad
    }
    delay(50);  // Wait a moment
}
