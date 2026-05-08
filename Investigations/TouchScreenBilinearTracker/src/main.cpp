#include <Arduino.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include "TouchPad.h"
#include "hwdefs.h"
#include "TouchScreen.h"
#include "DEBUG.h"

// The Adafruit 480x320 display pins
#define PIN_CS 10
#define PIN_DC 9
#define PIN_DRST 8
#define PIN_MOSI 11
#define PIN_DCLK 13  // Teensy 4.1
#define PIN_MISO 12

// Build the Pocket FT8 Transceivers TFT display object
HX8357_t3n gfx = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 TFT driver and pin definitions

// Build the TouchPad driver
TouchPad theTouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

// Build the TouchScreen calibrator
TouchScreen touchScreen(theTouchPad, gfx);

/**
 * @brief Display specified target
 * @param nodeIndex Target index
 */
void displayTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= touchScreen.getNTargets()) return;

    // Display target
    TouchScreenPoint p = touchScreen.getTargetCoordinate(nodeIndex);  // Screen coordinates for center of target
    gfx.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_YELLOW);   // Display target
}  // displayTarget()

/**
 * @brief Erase displayed target
 * @param nodeIndex Target index
 */
void eraseTarget(unsigned nodeIndex) {
    // Sanity check
    if (nodeIndex >= touchScreen.getNTargets()) return;

    // Erase target
    TouchScreenPoint p = touchScreen.getTargetCoordinate(nodeIndex);  // Screen coordinates for center of target
    gfx.fillCircle((unsigned)p.x, (unsigned)p.y, 2, HX8357_BLACK);    // Display target
}  // displayTarget()

/**
 * @brief Helper to Convert a TouchPadPoint into a TouchScreenPoint
 * @param p TouchPadPoint
 * @return TouchScreenPoint
 *
 * @note The touchpad hardware works with integer types while calibrator works with floats
 */
TouchScreenPoint toTCPoint(TouchPadPoint p) {
    TouchScreenPoint result;
    result.x = p.x;
    result.y = p.y;
    return result;
}

/**
 * @brief Blocking read of touchpad ADC coordinates
 * @return ADC coordinates as a TouchScreenPoint type
 */
TouchScreenPoint readTouchPad(void) {
    TouchPadPoint p;
    bool ok;
    do {
        ok = theTouchPad.readFiltered(p);  // Read touchpad...
    } while (!ok);  //...until we get valid coordinates
    return toTCPoint(p);  // Return valid coordinates as floats
}

/**
 * @brief Wait for touch/drag event to complete
 *
 * @note A touch event ends when the operator lifts the stylus from the pad
 */
void waitForTouchEnd(void) {
    TouchPadPoint p;
    while (!theTouchPad.readRaw(p)) {
        delay(50);
    }
}

/**
 * @brief Arduino initialization
 */
void setup() {
    Serial.begin(115200);
    delay(500);  // Wait for PlatformIO serial
    Serial.printf("Starting...\n");

    // Get the display running
    gfx.begin();
    gfx.fillScreen(HX8357_BLACK);  // Erase screen
    gfx.setRotation(0);

    // Calibrate the touchscreen
    touchScreen.begin();
    touchScreen.calibrate();

    // Erase screen before proceeding
    gfx.fillScreen(HX8357_BLACK);
}

/**
 * @brief Track touch events with displayed dots across the screen
 */
void loop() {
    bool ok;
    TouchScreenPoint screen;  // Corrected screen coordinates
    gfx.setRotation(3);       // GFX rotation
    ok = touchScreen.readTouchEvent(screen);
    if (ok) {  // Did we actually get anything?
        DPRINTF("screen: (%f,%f)\n", screen.x, screen.y);
        gfx.fillCircle((int)screen.x, (int)screen.y, 2, HX8357_YELLOW);  // Display corrected screen coordinates as dots
        waitForTouchEnd();                                               // Wait for drag to end
    }
    delay(50);
}
