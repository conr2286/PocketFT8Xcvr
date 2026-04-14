// ------------------------------------------------------------
// Teensy 4.1 Resistive Touch Driver (4-wire)
// ------------------------------------------------------------

#include <Arduino.h>
#include "touch.h"

// --- USER CONFIG: TOUCH PANEL PINS (Teensy 4.1 GPIO numbers) ---
static const int PIN_XP = 39;  // X+
static const int PIN_XM = 40;  // X− (A16)
static const int PIN_YP = 41;  // Y+ (A17)
static const int PIN_YM = 36;  // Y−
static const int PIN_XR = 37;  // X-Axis 510 Ohm resistor
static const int PIN_YR = 38;  // Y-Axis 510 Ohm resistor

// --- USER CONFIG: ADC / FILTERING ---
static const int ADC_BITS = 12;
static const int ADC_MAX = (1 << ADC_BITS) - 1;
static const int OVERSAMPLE_COUNT = 8;  // per axis
static const int Z_OVERSAMPLE = 4;
static const int SETTLE_US_AXIS = 100;   // after drive change
static const int SETTLE_US_SWITCH = 50;  // between reads
static const int Z_MIN_TOUCH = 150;      // reject very light touches
static const int Z_STABILITY_THR = 40;   // reject unstable pressure

// ------------------------------------------------------------
// Utility: median of small array
// ------------------------------------------------------------
template <size_t N>
static uint16_t median_u16(uint16_t (&v)[N]) {
    // simple insertion sort (N is small)
    for (size_t i = 1; i < N; ++i) {
        uint16_t key = v[i];
        size_t j = i;
        while (j > 0 && v[j - 1] > key) {
            v[j] = v[j - 1];
            --j;
        }
        v[j] = key;
    }
    return v[N / 2];
}

// ------------------------------------------------------------
// Low-level helpers: configure pins
// ------------------------------------------------------------
static void pinAnalogInput(int pin) {
    // Disable digital buffer, enable analog
    pinMode(pin, INPUT_DISABLE);
}

static void pinDriveHigh(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

static void pinDriveLow(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

static void pinHiZ(int pin) {
    pinMode(pin, INPUT_DISABLE);
}

// ------------------------------------------------------------
// ADC setup
// ------------------------------------------------------------
void touchInit() {
    analogReadResolution(ADC_BITS);
    analogReadAveraging(1);  // we do our own oversampling
    // analogReadFrequency(...) can be left default; we rely on delays
}

// ------------------------------------------------------------
// Read X axis (drive Y, sense X+)
// Also returns Z1/Z2 for pressure
// ------------------------------------------------------------
static void readRawX(uint16_t& x, uint16_t& z1, uint16_t& z2) {
    // Drive Y+ high, Y− low
    pinDriveHigh(PIN_YP);
    pinDriveLow(PIN_YM);

    // X+ and X− as analog inputs (high-Z)
    pinAnalogInput(PIN_XP);
    pinAnalogInput(PIN_XM);

    delayMicroseconds(SETTLE_US_AXIS);

    // Oversample X+
    uint16_t bufX[OVERSAMPLE_COUNT];
    for (int i = 0; i < OVERSAMPLE_COUNT; ++i) {
        bufX[i] = analogRead(PIN_XP);
        delayMicroseconds(SETTLE_US_SWITCH);
    }
    x = median_u16(bufX);

    // // Z1 = voltage at X− when Y is driven
    // uint16_t bufZ1[Z_OVERSAMPLE];
    // for (int i = 0; i < Z_OVERSAMPLE; ++i) {
    //     bufZ1[i] = analogRead(PIN_XM);
    //     delayMicroseconds(SETTLE_US_SWITCH);
    // }
    // z1 = median_u16(bufZ1);

    // // Z2 = voltage at X+ when  is driven
    // // Temporarily read YM as analog
    // pinAnalogInput(PIN_YM);
    // uint16_t bufZ2[Z_OVERSAMPLE];
    // for (int i = 0; i < Z_OVERSAMPLE; ++i) {
    //     bufZ2[i] = analogRead(PIN_YM);
    //     delayMicroseconds(SETTLE_US_SWITCH);
    // }
    // z2 = median_u16(bufZ2);

    // Restore YM drive low for next cycle
    pinDriveLow(PIN_YM);

    // Leave XP/XM as Hi-Z analog inputs for now
}

// ------------------------------------------------------------
// Read Y axis (drive X, sense Y+)
// ------------------------------------------------------------
static void readRawY(uint16_t& y) {
    // Drive X+ high, X− low
    pinDriveHigh(PIN_XP);
    pinDriveLow(PIN_XM);

    // Y+ as analog input, Y− Hi-Z
    pinAnalogInput(PIN_YP);
    pinHiZ(PIN_YM);

    delayMicroseconds(SETTLE_US_AXIS);

    uint16_t bufY[OVERSAMPLE_COUNT];
    for (int i = 0; i < OVERSAMPLE_COUNT; ++i) {
        bufY[i] = analogRead(PIN_YP);
        delayMicroseconds(SETTLE_US_SWITCH);
    }
    y = median_u16(bufY);
}

// ------------------------------------------------------------
// Compute a simple pressure proxy from Z1/Z2
// (not absolute ohms, just monotonic with pressure)
// ------------------------------------------------------------
static uint16_t computeZ(uint16_t z1, uint16_t z2) {
    // Classic formula uses Rx * (Z2/Z1 - 1); we just keep it simple:
    if (z1 == 0) return 0;
    int32_t z = (int32_t)z2 - (int32_t)z1;
    if (z < 0) z = 0;
    if (z > ADC_MAX) z = ADC_MAX;
    return (uint16_t)z;
}

// ------------------------------------------------------------
// Public: read one touch sample
// ------------------------------------------------------------
TouchSample touchRead() {
    TouchSample s{};
    s.valid = false;

    uint16_t xRaw, yRaw, z1, z2;

    // --- Phase 1: read X + Z1/Z2 ---
    readRawX(xRaw, z1, z2);
    uint16_t z = computeZ(z1, z2);

    // // Quick reject: no real touch
    // if (z < Z_MIN_TOUCH) {
    //     // Put everything Hi-Z
    //     pinHiZ(PIN_XP);
    //     pinHiZ(PIN_XM);
    //     pinHiZ(PIN_YP);
    //     pinHiZ(PIN_YM);
    //     return s;
    // }

    // --- Phase 2: read Y ---
    readRawY(yRaw);

    // Serial.printf("xRaw=%d yRaw=%d\n", xRaw, yRaw);

    // Optional: simple Z stability check (re-read Z quickly)
    uint16_t z1b, z2b;
    uint16_t xDummy;
    readRawX(xDummy, z1b, z2b);
    uint16_t zB = computeZ(z1b, z2b);

    if (abs((int)z - (int)zB) > Z_STABILITY_THR) {
        // unstable contact → reject
        pinHiZ(PIN_XP);
        pinHiZ(PIN_XM);
        pinHiZ(PIN_YP);
        pinHiZ(PIN_YM);
        return s;
    }

    // All good
    s.x = xRaw;
    s.y = yRaw;
    s.z = (z + zB) / 2;
    s.valid = true;

    // Neutral state
    pinHiZ(PIN_XP);
    pinHiZ(PIN_XM);
    pinHiZ(PIN_YP);
    pinHiZ(PIN_YM);

    return s;
}
