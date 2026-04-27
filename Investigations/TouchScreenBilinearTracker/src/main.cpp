#include <Arduino.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include "TouchPad.h"
#include "hwdefs.h"
#include "TouchCalibrator.h"
#include "DEBUG.h"

// The Adafruit 480x320 display pins
#define PIN_CS 10
#define PIN_DC 9
#define PIN_DRST 8
#define PIN_MOSI 11
#define PIN_DCLK 13  // Teensy 4.1
#define PIN_MISO 12

// Build the Pocket FT8 Transceivers TFT display object
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 TFT driver and pin definitions

// Build the touchscreen driver
TouchPad theTouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

// Build the touchscreen calibrator
TouchCalibrator touchCalibrator;

/**
 * @brief Display specified target
 * @param nodeIndex Target index
 */
void displayTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= touchCalibrator.getNTargets()) return;

    // Display target
    TCPoint p = touchCalibrator.getTargetCoordinate(nodeIndex);      // Screen coordinates for center of target
    tft.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_YELLOW);  // Display target
}  // displayTarget()

/**
 * @brief Erase displayed target
 * @param nodeIndex Target index
 */
void eraseTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= touchCalibrator.getNTargets()) return;

    // Erase target
    TCPoint p = touchCalibrator.getTargetCoordinate(nodeIndex);     // Screen coordinates for center of target
    tft.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_BLACK);  // Display target
}  // displayTarget()

/**
 * @brief Helper to Convert a TouchPadPoint into a TCPoint
 * @param p TouchPadPoint
 * @return TCPoint
 *
 * @note The touchpad hardware works with integer types while calibrator works with floats
 */
TCPoint toTCPoint(TouchPadPoint p) {
    TCPoint result;
    result.x = p.x;
    result.y = p.y;
    return result;
}

/**
 * @brief Blocking read of touchpad ADC coordinates
 * @return ADC coordinates as a TCPoint type
 */
TCPoint readTouchPad(void) {
    TouchPadPoint p;
    do {
        p = theTouchPad.getTouchEvent();  // Read touchpad...
    } while (p.state == TS_NO_TOUCH);  //...until we get valid coordinates
    return toTCPoint(p);  // Return valid coordinates as floats
}

/**
 * @brief Wait for touch/drag event to complete
 *
 * @note A touch event ends when the operator lifts the stylus from the pad
 */
void waitForTouchEnd(void) {
    while (theTouchPad.getTouchPoint().state != TS_NO_TOUCH) {
        delay(50);
    }
}

/**
 * @brief Arduino initialization
 */
void setup() {
    Serial.begin(115200);
    Serial.printf("Starting...\n");

    // Get the display running
    tft.begin();
    tft.fillScreen(HX8357_BLACK);  // Erase screen

    // Touchscreen calibration
    unsigned nTargets = touchCalibrator.getNTargets();
    for (unsigned nodeIndex = 0; nodeIndex < nTargets; nodeIndex++) {
        char s[256];

        // Prompt the operator
        tft.setCursor(50, 100);
        sprintf(s, "Touch target %d of %d\n", nodeIndex + 1, nTargets);  // Prompt msg
        tft.print(s);
        displayTarget(nodeIndex);  // Display the target for operator to touch

        // Read and record the calibration data
        TCPoint adc = readTouchPad();                           // Read touchpad coordinates as ADC values
        touchCalibrator.recordCalibrationNode(nodeIndex, adc);  // Record info for this node
        waitForTouchEnd();                                      // Wait for operator to quit dragging stylus on the touchpad
        eraseTarget(nodeIndex);                                 // Erase target from display
        delay(200);                                             // Don't rush the operator
    }

    // Erase screen before proceeding
    tft.fillScreen(HX8357_BLACK);
}

/**
 * @brief Track touch events with displayed dots across the screen
 */
void loop() {
    TouchPadPoint p = theTouchPad.getTouchEvent();    // Read the touchpad
    if (p.state != TS_NO_TOUCH) {                     // Did we actually get anything?
        TCPoint raw = toTCPoint(p);                   // Change coordinates to float
        TCPoint screen;                               // Corrected screen coordinates
        touchCalibrator.mapRawToScreen(raw, screen);  // Apply calibration
        DPRINTF("raw (%f,%f) mapped to screen (%f,%f)\n", raw.x, raw.y, screen.x, screen.y);
        tft.fillCircle((int)screen.x, (int)screen.y, 2, HX8357_YELLOW);  // Display corrected coordinates
        waitForTouchEnd();                                               // Wait for drag to end
    }
    delay(50);
}
