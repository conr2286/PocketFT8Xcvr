/**
 * Program to produce a TouchScreen calibration data file, TOUCHSCR.DAT, on SD
 *
 * USAGE:
 *  Restores the TouchScreen calibration data from the TOUCHSCR.DAT file if available.
 *  Else,  guides the operator through a calibration exercise using 9 targets.
 *  Once calibration has been achieved, erases the screen and displays operator's
 *  touchpoints.
 *
 * DISCUSSION:
 *  + Developed using PlatformIO targeting the Teensy 4.1 Arduino environment
 *  + Developed for the Adafruit P2050 TFT resistive touchscreen display
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include "TouchPad.h"
#include "AGUI.h"
#include "FT8Font.h"  //Customized font for the Pocket FT8 Revisited
#include "AWidget.h"
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

// Calibration data (serialized) file name
#define CALIBRATION_DATA_FILENAME "TOUCHSCR.DAT"

// Build the Pocket FT8 Transceivers TFT display object
HX8357_t3n gfx = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 TFT driver and pin definitions

// Build the TouchPad driver
TouchPad theTouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

// Build the TouchScreen calibrator
TouchScreen touchScreen(theTouchPad, gfx);

// Build the GUI
static AGUI& agui = AGUI::getInstance(gfx, 3, FT8Font);

/**
 * @brief Arduino initialization
 */
void setup() {
    File calibFile;

    // Start serial connection with PlatformIO for debugging
    Serial.begin(115200);
    delay(100);                      // Wait for PlatformIO serial
    Serial.printf("Starting...\n");  // Always announce alive-and-well... so far

    // Get the I2C bus's running
    Wire.setSDA(PIN_SDA);
    Wire.setSCL(PIN_SCL);
    Wire.begin();

    Wire1.setSDA(PIN_SDA2);
    Wire1.setSCL(PIN_SCL2);
    Wire1.begin();

    // Get the SD file system running
    if (!SD.begin(BUILTIN_SDCARD)) {
        DTRACE();
    }

    // Get the TFT display running
    gfx.begin();
    gfx.fillScreen(HX8357_BLACK);  // Erase screen
    gfx.setRotation(0);

    // Get the TouchScreen package running
    touchScreen.begin();

    // Attempt to restore calibration data from SD file
    if (calibFile = SD.open(CALIBRATION_DATA_FILENAME, FILE_READ)) {
        DTRACE();
        touchScreen.deserialize(calibFile);
        calibFile.close();
    }

    // If unable to restore calibration from file then have operator [re]calibrate the touchscreen
    if (!touchScreen.isCalibrated()) {
        do {
            // Do the calibration
            touchScreen.doCalibration(agui);  // UI controls calibration steps

            // Serialize a successful calibration
            if (touchScreen.isCalibrated()) {
                DTRACE();

                File calibFile = SD.open(CALIBRATION_DATA_FILENAME, FILE_WRITE);
                if (calibFile) {
                    DTRACE();
                    if (!touchScreen.serialize(calibFile)) {
                        DTRACE();
                    }
                } else {
                    DTRACE();
                }
                calibFile.close();
            }

        } while (!touchScreen.isCalibrated());
    }
}

/**
 * @brief Nothing to do here
 */
void loop() {
    TouchScreenPoint p;
    delay(100);
    if (touchScreen.readTouchEvent(p)) {
        // Display where touch event landed on the screen
        agui.gfx->fillCircle(p.x, p.y, 2, A_YELLOW);
    }
}
