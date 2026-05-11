
#include <TouchScreen.h>
#include <AGUI.h>
#include <Arduino.h>
#include <AToggleButton.h>

// Define screen layout of buttons
static const unsigned buttonWidth = 50;
static const unsigned buttonHeight = 30;
static const unsigned buttonRowX = 100;  // Upper left corner of button bar
static const unsigned buttonRowY = 100;  // Upper left corner of button bar

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
    // Initialization
    unsigned displayWidth = agui.gfx.width();
    unsigned displayHeight = agui.gfx.height();

    // Build and display buttons on UI
    AToggleButton okButton = AToggleButton("OK", buttonRowX, buttonRowY, buttonWidth, buttonHeight);
    AToggleButton testButton = AToggleButton("TEST", buttonRowX + buttonWidth + buttonWidth / 3, buttonRowY);
    AToggleButton cancelButton = AToggleButton("CANCEL", buttonRowX + 2 * buttonWidth + 2 * (buttonWidth / 3), buttonRowY);

    // Erase the display and perform the calibration
    agui.fillRect(0, 0, displayWidth, displayHeight, A_BLACK);
}
