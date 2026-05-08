

#include <Arduino.h>
#include "TouchScreen.h"
#include "TouchPad.h"

/**
 * @brief Helper to display specified target on GFX screen
 * @param nodeIndex Target index
 */
void TouchScreen::displayTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= getNTargets()) return;

    // Display target
    TouchScreenPoint p = getTargetCoordinate(nodeIndex);             // Screen coordinates for center of target
    gfx.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_YELLOW);  // Display target
}  // displayTarget()

/**
 * @brief Erase displayed target
 * @param nodeIndex Target index
 */
void TouchScreen::eraseTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= getNTargets()) return;

    // Erase target
    TouchScreenPoint p = getTargetCoordinate(nodeIndex);            // Screen coordinates for center of target
    gfx.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_BLACK);  // Display target
}  // displayTarget()

/**
 * @brief Helper to Convert a TouchPadPoint into a TouchScreenPoint
 * @param p TouchPadPoint
 * @return TouchScreenPoint
 *
 * @note The touchpad hardware works with integer types while calibrator works with floats
 */
TouchScreenPoint TouchScreen::toTCPoint(TouchPadPoint p) {
    TouchScreenPoint result;
    result.x = p.x;
    result.y = p.y;
    return result;
}

/**
 * @brief Helper to perform blocking read of touchpad ADC coordinates
 * @return ADC coordinates as a TouchScreenPoint type
 */
TouchScreenPoint TouchScreen::readTouchPad(void) {
    TouchPadPoint p;
    bool ok;
    do {
        ok = touchPad.readFiltered(p);  // Read touchpad...
    } while (!ok);  //...until we get valid coordinates
    return toTCPoint(p);  // Return valid coordinates as floats
}

/**
 * @brief Helper to wait for touch/drag event to complete
 *
 * @note A touch event ends when the operator lifts the stylus from the pad
 */
void TouchScreen::waitForTouchEnd(void) {
    TouchPadPoint p;
    while (!touchPad.readRaw(p)) {
        delay(50);
    }
}

/**
 * @brief Calibrate the TouchScreen
 * @return true if successful
 *
 * DISCUSSION:
 *  Calibration results are stored in the calibration table but are not serialized to a file
 */
bool TouchScreen::calibrate(void) {
    unsigned nTargets = getNTargets();  // How many targets are required?

    // Loop executes once for each calibration target
    for (unsigned nodeIndex = 0; nodeIndex < nTargets; nodeIndex++) {
        char s[256];

        // Prompt the operator and display this target
        gfx.setCursor(50, 100);
        snprintf(s, sizeof(s), "Touch target %d of %d\n", nodeIndex + 1, nTargets);  // Prompt msg
        gfx.print(s);
        displayTarget(nodeIndex);  // Display the target for operator to touch

        // Read and record the calibration data for this target
        TouchScreenPoint adc = readTouchPad();  // Read touchpad coordinates as ADC values
        recordCalibrationNode(nodeIndex, adc);  // Record info for this node
        waitForTouchEnd();                      // Wait for operator to quit dragging stylus on the touchpad
        eraseTarget(nodeIndex);                 // Erase target from display
        delay(200);                             // Don't rush the operator
    }

    return true;
}