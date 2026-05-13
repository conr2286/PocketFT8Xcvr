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
    Serial.begin(115200);
    delay(1000);  // Wait for PlatformIO serial
    Serial.printf("Starting...\n");

    Wire.setSDA(PIN_SDA);
    Wire.setSCL(PIN_SCL);
    Wire.begin();

    Wire1.setSDA(PIN_SDA2);
    Wire1.setSCL(PIN_SCL2);
    Wire1.begin();

    // Get the display running
    gfx.begin();
    gfx.fillScreen(HX8357_BLACK);  // Erase screen
    gfx.setRotation(0);

    // Initialize the TouchScreen calibration package
    touchScreen.begin();

    // Do the calibration
    bool ok = false;

    do {
        // Do the calibration
        ok = touchScreen.doCalibration(agui);
        DPRINTF("doCalibration()=%d\n", ok);

        // Serialize successful calibration?
        if (ok) {
            DTRACE();
            // Get the SD file system running
            if (!SD.begin(BUILTIN_SDCARD)) {
                DTRACE();
            }
            File calibFile = SD.open("TOUCHSCR.DAT", FILE_WRITE);
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

    } while (!ok);
}

/**
 * @brief Track touch events with displayed dots across the screen
 */
void loop() {
}
