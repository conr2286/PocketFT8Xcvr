#include <Arduino.h>
#include <EEPROM.h>
#include <DEBUG.h>

#include "touch9.h"
#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include "hwdefs.h"
#include "TouchPad.h"

// ------------------------------------------------------------
// Replace this with your actual TFT driver
// Example: ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST);
// ------------------------------------------------------------

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

// Dummy stand‑in functions for this example:
void tft_fillScreen(uint16_t color) {
    tft.fillScreen(color);
}
void tft_drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    tft.drawLine(x0, y0, x1, y1, color);
}
void tft_setCursor(int x, int y) {
    tft.setCursor(x, y);
}
void tft_print(const char* s) {
    tft.print(s);
}

// ------------------------------------------------------------
// Draw calibration target callback
// ------------------------------------------------------------
void drawTarget(int index, const T9Point& p) {
    tft_fillScreen(0x0000);  // black

    // Crosshair
    // tft_drawLine(p.x - 12, p.y, p.x + 12, p.y, 0xFFFF);
    // tft_drawLine(p.x, p.y - 12, p.x, p.y + 12, 0xFFFF);
    tft.fillCircle(p.x, p.y, 6, HX8357_YELLOW);

    // Label
    tft_setCursor(10, 10);
    char buf[32];
    snprintf(buf, sizeof(buf), "Touch target %d of 9", index + 1);
    tft_print(buf);
}

// ------------------------------------------------------------
// Global calibration object
// ------------------------------------------------------------
T9Calib9 g_cal;

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(300);

    // Initialize display here
    tft.begin();
    tft.fillScreen(HX8357_BLACK);  // Erase screen

    // tft.setRotation(1);

    // EEPROM.begin(512);
    t9_init();

    Serial.println("Touch9: Initializing...");

    // Try loading calibration
    // if (!t9_calib_load(g_cal)) {
    if (true) {
        Serial.println("No valid calibration found. Starting calibration...");
        t9_calib_start(drawTarget);
    } else {
        Serial.println("Calibration loaded from EEPROM.");
    }
}

// ------------------------------------------------------------
// Loop
// ------------------------------------------------------------
void loop() {
    // --------------------------------------------------------
    // Calibration mode
    // --------------------------------------------------------
    if (t9_calib_state() == T9CalState::Running) {
        t9_calib_update();

        if (t9_calib_state() == T9CalState::Done) {
            tft.fillScreen(HX8357_BLACK);  // Erase screen

            DTRACE();
            g_cal = t9_calib_get();
            t9_calib_save();
            Serial.println("Calibration complete and saved.");
        }
        return;
    }

    // --------------------------------------------------------
    // Normal operation mode
    // --------------------------------------------------------
    T9Point raw;
    uint16_t z;

    if (t9_read_filtered(raw, z)==TS_TOUCH) {
        T9Point screen;
        DTRACE();
        if (t9_map_raw_to_screen(g_cal, raw, screen)) {
            DTRACE();

            // // Print mapped coordinates
            // Serial.print("Touch @ ");
            // Serial.print(screen.x);
            // Serial.print(", ");
            // Serial.println(screen.y);

            DPRINTF("Corrected touch:  %f,%f\n", screen.x, screen.y);

            // Example: draw a dot on the screen
            tft.fillCircle(screen.x, screen.y, 3, HX8357_CYAN);
        }
        delay(50);
    }
}
