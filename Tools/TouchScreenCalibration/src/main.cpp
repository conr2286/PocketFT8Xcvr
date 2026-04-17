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

// /**
//  * @brief Get the corrected TouchPoint value
//  * @param rawP The uncorrected TouchPoint in the unrotated screen coordinate system
//  * @return The corrected TouchPoint
//  *
//  * @note This function corrects for the margins and non-linearity across an axis.
//  */
// TouchPoint getCorrectedTouchPoint(TouchPoint rawP) {
//     TouchPoint result;
//     result.z = rawP.z;  // Return z unmodified
//     if (rawP.z) {
//         // DTRACE();
//         int nRow = rawP.y / TS_CORTAB_ROWS;  // Select a row in the correction table
//         int nCol = rawP.x / TS_CORTAB_COLS;  // Select a column in the correction table
//         DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
//         DPRINTF("using tsCorX = %d tsCorY = %d\n", tsCorX[nCol][nRow], tsCorY[nCol][nRow]);
//         result.x = rawP.x + tsCorX[nCol][nRow];  // Correct the x coordinate
//         result.y = rawP.y + tsCorY[nCol][nRow];  // Correct the y coordinate
//         DPRINTF("rawP.x=%d rawP.y=%d nRow=%d nCol=%d result.x=%d result.y=%d\n", rawP.x, rawP.y, nRow, nCol, result.x, result.y);
//     }
//     return result;
// }  // getCorrectedTouchPoint()

// int updateCorTab(TouchPoint expectedP, TouchPoint actualP) {
//     // Calculate the errors on both axis
//     int errX = actualP.x - expectedP.x;  // Error on x-Axis
//     int errY = actualP.y - expectedP.y;  // Error on y-Axis

//     // Update correction table from errors
//     int nRow = actualP.y / TS_CORTAB_ROWS;  // Select a row in the correction table
//     int nCol = actualP.x / TS_CORTAB_COLS;  // Select a column in the correction table

//     DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
//     tsCorX[nCol][nRow] -= errX;  // Update the x-Axis correction
//     tsCorY[nCol][nRow] -= errY;  // Update the y-Axis correction
//     DPRINTF("updated tsCorX = %d tsCorY = %d\n", tsCorX[nCol][nRow], tsCorY[nCol][nRow]);

//     // Return the root mean square of the x/y error
//     int errRMS = sqrt(errX * errX + errY * errY);  // Root mean square of errors
//     DPRINTF("expectedP.x=%d actualP.x=%d errX=%d   expectedP.y=%d actualP.y=%d errY=%d   errRMS=%d\n", expectedP.x, actualP.x, errX, expectedP.y, actualP.y, errY, errRMS);
//     DPRINTF("confirm nRow=%d nCol=%d\n", nRow, nCol);
//     return errRMS;
// }

// /**
//  * @brief Exercises a single target on the touchscreen
//  * @param x Target's x-Coord
//  * @param y Target's y-Coord
//  * @return Touchpoint deviation (in pixels) from target coordinates
//  */
// unsigned exerciseTouchTarget(int x, int y) {
//     TouchPoint rawP;  // Raw display system coordinates of touchpoint
//     TouchPoint corP;  // Corrected display system coordinates of touchpoint

//     DPRINTF("\n");

//     // Erase screen and display operator prompt and touch target
//     tft.setCursor(x, y);                                   // Display the target
//     tft.fillCircle(x, y, TS_ACCURACY / 2, HX8357_YELLOW);  // Draw the target

//     // Wait for the operator to touch the target
//     while (rawP.z == 0) {
//         rawP = touchPad.getTouchPoint();
//         if (rawP.z == 0) delay(10);
//     }
//     DPRINTF("rawP.x=%d rawP.y=%d\n", rawP.x, rawP.y);

//     // Apply existing correction and calculate error
//     corP = getCorrectedTouchPoint(rawP);  // Apply the correction factor

//     // Update correction tables from current error
//     TouchPoint expectedP;
//     expectedP.x = x;
//     expectedP.y = y;
//     expectedP.z = 1;
//     int errRMS = updateCorTab(expectedP, corP);

//     // Remove target from display
//     tft.fillCircle(x, y, TS_ACCURACY / 2, HX8357_BLACK);
//     delay(200);

//     // Return the RMS error
//     return errRMS;
// }  // exerciseTouchTarget()

// /**
//  * @brief Debugging function to print the correction tables
//  * @param x1 Upper-left corner of bounding rectangle
//  * @param y1 Upper-left corner of bounding rectangle
//  * @param x2 Lower-right corner of bounding rectangle
//  * @param y2 Lower-right corner of bounding rectangle
//  */
// void dumpCorTabs(int x1, int y1, int x2, int y2) {
//     int x, y;
//     for (y = y1; y <= y2; y++) {
//         for (x = x1; x <= x2; x++) {
//             Serial.printf("(%d,%d) ", tsCorX[x][y]);
//         }
//         Serial.printf("\n");
//     }
//     Serial.printf("\n");
// }

// /**
//  * @brief Fills the interior of a bounding rectangle within tsCorX[][] with interpolated values
//  * @param x1 Rectangle's upper left side x-Coord
//  * @param y1 Rectangle's upper left y-Coord
//  * @param x2 Rectangle's lower right x-Coord
//  * @param y2 Rectangle's lower right y-Coord
//  */
// void interpolateCorTab(int xs1, int ys1, int xs2, int ys2) {
//     int x;     // Iterates over columns
//     int y;     // Iterates over rows
//     float dy;  // Delta correction value over rows
//     float ay;  // Accumulated correction value over rows

//     DPRINTF("xs1=%d ys1=%d  xs2=%d ys2=%d\n", xs1, ys1, xs2, ys2);

//     // Convert screen coordinates to correction table indices
//     int x1 = xs1 / TS_ACCURACY;
//     int y1 = ys1 / TS_ACCURACY;
//     int x2 = xs2 / TS_ACCURACY;
//     int y2 = ys2 / TS_ACCURACY;

//     // Sanity checks
//     DPRINTF("x1=%d y1=%d  x2=%d y2=%d\n", x1, y1, x2, y2);
//     if ((x1 >= x2) || (y1 >= y2)) return;
//     // DTRACE();
//     if ((x2 > TS_CORTAB_COLS) || (y2 > TS_CORTAB_ROWS)) return;
//     // DTRACE();
//     if ((x1 < 0) || (y1 < 0)) return;
//     // DTRACE();
//     if ((x2 - x1) < 3 || (y2 - y1) < 3) return;
//     DTRACE();

//     /*********************************
//      * Interpolate cells in tsCorX[][]
//      ********************************/

//     // Calculate tsCorX variance along y-Axis on left edge of rectangle
//     dy = (tsCorX[x1][y2] - tsCorX[x1][y1]) / (TS_CORTAB_ROWS - 1);  // Variance between rows on left edge

//     // Interpolate tsCorX cells along the left edge of bounding rectangle
//     ay = tsCorX[x1][y1];             // Correction anchored by top left corner
//     for (y = y1 + 1; y < y2; y++) {  // Visit cells between but excluding top and bottom
//         ay += dy;                    // Calc correction for this row
//         tsCorX[x1][y] = ay;          // Record interpolated value
//     }

//     // Calculate tsCorX variance along y-Axis on right edge of bounding rectangle
//     dy = (tsCorX[x2][y2] - tsCorX[x2][y1]) / (TS_CORTAB_ROWS - 1);  // Variance between rows on right edge

//     // Interpolate tsCorX cells along the right edge of rectangle
//     ay = tsCorX[x2][y1];             // Correction anchored by top right corner
//     for (y = y1 + 1; y < y2; y++) {  // Visit cells between but excluding top and bottom
//         ay += dy;                    // Calc correction for this row
//         tsCorX[x2][y] = ay;          // Record interpolated value
//     }

//     // Interpolate tsCorX cells between the left and right edge of each row
//     for (y = y1; y <= y2; y++) {  // Iterate over each row *including* the top and bottom

//         // Calculate variance along the x-Axis of row y
//         dy = (tsCorX[x2][y] - tsCorX[x1][y]) / (TS_CORTAB_COLS - 1);  // Variance between cells along this row

//         // Interpolate cells between the left and right edges of this row y
//         ay = tsCorX[x1][y];              // Correction anchored by left corner of this row y
//         for (x = x1 + 1; x < x2; x++) {  // Visit each cell between but excluding left and right edges of this row
//             ay += dy;                    // Calc correction for this col in this row
//             tsCorX[x][y] = ay;           // Record interpolated value in cell
//         }
//     }

//     /*********************************
//      * Interpolate cells in tsCorY[][]
//      ********************************/

//     // Calculate tsCorY variance along y-Axis on left edge of rectangle
//     dy = (tsCorY[x1][y2] - tsCorY[x1][y1]) / (TS_CORTAB_ROWS - 1);  // Variance between rows on left edge

//     // Interpolate tsCorY cells along the left edge of bounding rectangle
//     ay = tsCorY[x1][y1];             // Correction anchored by top left corner
//     for (y = y1 + 1; y < y2; y++) {  // Visit cells between but excluding top and bottom
//         ay += dy;                    // Calc correction for this row
//         tsCorY[x1][y] = ay;          // Record interpolated value
//     }

//     // Calculate tsCorY variance along y-Axis on right edge of bounding rectangle
//     dy = (tsCorY[x2][y2] - tsCorY[x2][y1]) / (TS_CORTAB_ROWS - 1);  // Variance between rows on right edge

//     // Interpolate tsCorY cells along the right edge of rectangle
//     ay = tsCorY[x2][y1];             // Correction anchored by top right corner
//     for (y = y1 + 1; y < y2; y++) {  // Visit cells between but excluding top and bottom
//         ay += dy;                    // Calc correction for this row
//         tsCorY[x2][y] = ay;          // Record interpolated value
//     }

//     // Interpolate tsCorY cells between the left and right edge of each row
//     for (y = y1; y <= y2; y++) {  // Iterate over each row *including* the top and bottom

//         // Calculate variance along the x-Axis of row y
//         dy = (tsCorY[x2][y] - tsCorY[x1][y]) / (TS_CORTAB_COLS - 1);  // Variance between cells along this row

//         // Interpolate cells between the left and right edges of this row y
//         ay = tsCorY[x1][y];              // Correction anchored by left corner of this row y
//         for (x = x1 + 1; x < x2; x++) {  // Visit each cell between but excluding left and right edges of this row
//             ay += dy;                    // Calc correction for this col in this row
//             tsCorY[x][y] = ay;           // Record interpolated value in cell
//         }
//     }

//     // Debugging
//     dumpCorTabs(x1, y1, x2, y2);
// }

/**
 * @brief Read coordinates of the next touch event
 * @return TouchPoint coordinates
 *
 * @note This is a blocking read of the next touch event
 *
 * TODO:  If we had an isTouching state variable, we could optimize by only
 * waiting for operator to pick-up the stylus prior to the next read
 */
TouchPoint getNextTouchPoint(void) {
    TouchPoint result;

    // Wait for operator to touch stylus to the pad
    do {
        delay(10);                          // Wait a moment
        result = touchPad.getTouchPoint();  // Try to read the touchpad
    } while (result.z == 0);  // Wait for valid touch event

    // Wait for operator to remove the stylus from pad
    while (touchPad.getTouchPoint().z) {
        delay(10);  // Wait a moment
    }

    return result;

}  // getNextTouchPoint()

/**
 * @brief Displays a prompt message for operator on the TFT
 * @param x Screen coordinate where target will appear
 * @param y Screen coordinate where target will appear
 */
void promptOperator(unsigned x, unsigned y) {
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

    // Select a row and column in the correction matrices
    unsigned corRow = p.y / TS_CORTAB_ROWS;
    unsigned corCol = p.x / TS_CORTAB_COLS;

    // Sanity checks
    if ((corRow >= TS_CORTAB_ROWS) || (corCol >= TS_CORTAB_COLS) || (p.z == 0)) {
        DTRACE();
        c.x = c.y = c.z = 0;  // Return error indication
    }

    // Apply correction matrices to uncorrected coordinate
    c.x = p.x + tsCorX[p.x][p.y];  // x-Axis correction from tsCorX column x, row y
    c.y = p.y + tsCorY[p.x][p.y];  // y-Axis correction from tsCorY column x, row y
    c.z = p.z;
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
    TouchPoint p1 = getNextTouchPoint();               // Read the touchpad
    if (p1.z) {                                        // Check for valid touch event
        TouchPoint p2 = getCorrectedPoint(p1);         // Map p1 to p2 using interpolation matrices
        tft.fillCircle(p2.x, p2.y, 3, HX8357_YELLOW);  // Show operator where we think the stylus touched the pad
    }
    delay(10);  // Wait a moment
}
