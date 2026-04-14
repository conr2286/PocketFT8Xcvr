#include <Arduino.h>

#include "Adafruit_GFX.h"
#include <Adafruit_HX8357.h>
#include "touch.h"

// The Adafruit 480x320 display pins
#define PIN_CS 10
#define PIN_DC 9
#define PIN_DRST 8
#define PIN_MOSI 11
#define PIN_DCLK 13  // Teensy 4.1
#define PIN_MISO 12

Adafruit_HX8357 tft = Adafruit_HX8357(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins

void setup() {
    Serial.begin(115200);
    Serial.printf("Starting...\n");
    touchInit();
}

void loop() {
    TouchSample t = touchRead();
    if (t.valid) {
        Serial.printf("X=%d Y=%d Z=%d\n", t.x, t.y, t.z);
    }
    delay(100);
}
