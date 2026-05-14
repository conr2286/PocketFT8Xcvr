
#include <TouchScreen.h>
#include <AGUI.h>
#include <Arduino.h>
#include <AToggleButton.h>
#include "NODEBUG.h"

// Define screen layout of buttons
static const unsigned buttonWidth = 80;
static const unsigned buttonHeight = 30;
static const unsigned buttonRowX = 20;     // Upper left corner of button bar
static const unsigned buttonRowY = 100;    // Upper left corner of button bar
static const unsigned buttonSpacing = 20;  // Space between buttons

/**
 * @brief User Interface for controlling the TouchScreen calibration
 * @param agui Reference to the GUI object
 * @return true if confirmed successful, false if not
 *
 * DISCUSSION:
 *  Provides a User Interface to perform a TouchScreen calibration, prompting operator to
 *  touch the 9 calibration targets, test the calibration, and confirm/reject the calibration.
 *
 *  doCalibration() does not serialize nor deserialize the TouchScreen calibration, but it
 *  informs the caller if the calibration was deemed successful.  doCalibration() performs
 *  a single calibration attempt which the caller may choose to retry if unsuccessful.
 *
 *  doCalibration() is designed to be called from the Arduino setup() function.
 *
 *  Prior to invoking doCalibration(), caller should:
 *      + Instantiate Adafruit GFX (HX8357_t3n on Teensy)
 *      + Instantiate AGUI (Another GUI)
 *      + Instantiate TouchPad driver
 *      + Instantiate TouchScreen calibrator
 *      + Invoke TouchScreen.begin()
 */
bool TouchScreen::doCalibration(AGUI& agui) {
    // Sanity checks
    if (!initialized) {
        DTRACE();
        return false;
    }

    // Initialization
    unsigned saveRotation = agui.gfx->getRotation();  // GFX Display rotation
    agui.gfx->setRotation(0);                         // We need display coords aligned with touchpad HW coordinates
    unsigned displayWidth = agui.gfx->width();        // Width pixels in default rotation
    unsigned displayHeight = agui.gfx->height();      // Height pixels in default rotation

    // Erase the display and perform a calibration
    agui.fillRect(0, 0, displayWidth, displayHeight, A_BLACK);
    calibrate();
    agui.fillRect(0, 0, displayWidth, displayHeight, A_BLACK);

    // Build and display buttons on UI
    AToggleButton okButton = AToggleButton("OK", buttonRowX, buttonRowY, buttonWidth, buttonHeight);
    AToggleButton testButton = AToggleButton("TEST", buttonRowX + buttonWidth + buttonSpacing, buttonRowY, buttonWidth, buttonHeight);
    AToggleButton cancelButton = AToggleButton("CANCEL", buttonRowX + 2 * buttonWidth + 2 * buttonSpacing, buttonRowY, buttonWidth, buttonHeight);
    delay(100);

    // Define the operator's commands
    enum class Command { NONE,
                         OK,
                         TEST,
                         CANCEL } cmd;

    // Outer loop allows operator to repeatedly test the calibration at a point of their choosing
    do {
        TouchScreenPoint p;  // Operator's touch screen coordinates
        cmd = Command::NONE;

        // Await the operator's command
        do {
            if (readTouchEvent(p)) {
                DPRINTF("TOUCHED p.x=%d p.y=%d\n", (unsigned)p.x, (unsigned)p.y);
                // Discern operator's cmd by polling buttons with touch screen coordinates
                if (okButton.isWithin(p.x, p.y))
                    cmd = Command::OK;
                else if (testButton.isWithin(p.x, p.y))
                    cmd = Command::TEST;
                else if (cancelButton.isWithin(p.x, p.y))
                    cmd = Command::CANCEL;
                else
                    delay(100);
                DPRINTF("cmd=%u\n", static_cast<unsigned>(cmd));
            }
        } while (cmd == Command::NONE);

        // Does operator want to test the calibration (again)
        DPRINTF("cmd=%u\n", static_cast<unsigned>(cmd));
        if (cmd == Command::TEST) {
            delay(200);
            // Wait for the operator to touch a test location on the screen
            while (!readTouchEvent(p)) {
                delay(100);  // Wait a moment
            }

            // Display where touch event landed on the screen
            agui.gfx->fillCircle(p.x, p.y, 2, A_YELLOW);
        }
        DPRINTF("cmd=%u\n", static_cast<unsigned>(cmd));

    } while (cmd == Command::TEST);

    // Restore rotation and return result
    agui.gfx->setRotation(saveRotation);
    DTRACE();

    calibrated = (cmd == Command::OK);
    return calibrated;

}  // doCalibration()
