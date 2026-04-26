#include <Arduino.h>

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"
#include "TouchPad.h"
#include "hwdefs.h"

// The Adafruit 480x320 display pins
#define PIN_CS 10
#define PIN_DC 9
#define PIN_DRST 8
#define PIN_MOSI 11
#define PIN_DCLK 13  // Teensy 4.1
#define PIN_MISO 12

HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
static TouchPad touchPad = TouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);

void setup() {
    Serial.begin(115200);
    Serial.printf("Starting...\n");

    tft.begin();
    tft.fillScreen(HX8357_BLACK);  // Erase screen
}

/**
 * @brief Displayed dots track touch events across the screen
 */
void loop() {
    static int minX = 9999, maxX = 0, minY = 9999, maxY = 0;

    TouchPoint t = touchPad.getTouchEvent();

    // Process touching events
    if (t.z) {
        Serial.printf("X=%d Y=%d Z=%d  minX=%d minY=%d maxX=%d maxY=%d\n", t.x, t.y, t.z, minX, minY, maxX, maxY);

        // Track observed min/max x/y ADC values
        if (t.x < minX) minX = t.x;
        if (t.x > maxX) maxX = t.x;
        if (t.y < minY) minY = t.y;
        if (t.y > maxY) maxY = t.y;

        // Linearly map ADC coordinates into screen coordinates based upon known min/max values
        int cx = map(t.x, 132, 780, 0, 319);
        int cy = map(t.y, 54, 958, 0, 479);
        tft.fillCircle(cx, cy, 2, HX8357_WHITE);
    }
    delay(100);
}
