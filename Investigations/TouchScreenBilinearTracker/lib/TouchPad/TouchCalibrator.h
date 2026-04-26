#pragma once

#include <Arduino.h>

// ---------- Basic types ----------

struct TCPoint {
    float x;
    float y;
};

// The touchpad is calibrated at 9 on-screen targets known as "nodes" each
// associating a raw ADC coordinate tuple with a screen coordinate tuple.
struct TouchCalibrationNode {
    TCPoint raw;     // raw ADC reading
    TCPoint screen;  // ideal screen coordinate
};

// ---------- Public configuration ----------

// Adjust these to your wiring
// namespace T9Pins {
// // Resistive touch pins
// constexpr int PIN_XP = 33;  // example
// constexpr int PIN_XM = 32;
// constexpr int PIN_YP = 31;
// constexpr int PIN_YM = 30;
// }  // namespace T9Pins

// Screen resolution
// constexpr int T9_SCREEN_WIDTH = 320;
// constexpr int T9_SCREEN_HEIGHT = 480;

// Record the calibration table as a linear array of bindings between ADC and screen coordinates
struct TouchCalibrationTable {
    TouchCalibrationNode nodes[9];  // row-major 3x3
};

// Getters and setters
unsigned getNTargets(void);
TCPoint getTargetCoordinate(unsigned idx);
void recordCalibrationNode(unsigned idx, TCPoint adc);

// ---------- Driver API ----------

void t9_init();

// Returns true if a touch is detected and fills raw (0..1023)
bool t9_read_raw(TCPoint& raw, uint16_t& z);

// Filtered raw (median/oversampled). Returns true if touch present.
bool t9_read_filtered(TCPoint& raw, uint16_t& z);

// ---------- Calibration API ----------

enum class T9CalState {
    Idle,
    Running,
    Done
};

// Callback type: draw target i (0..8) at given screen coordinate
using T9DrawTargetFn = void (*)(int index, const TCPoint& screenPos);

// Start 9-point calibration
void t9_calib_start(T9DrawTargetFn drawFn);

// Call periodically (e.g. in loop)
void t9_calib_update();

// Get calibration state
T9CalState t9_calib_state();

// Get final calibration (valid when state == Done)
const TouchCalibrationTable& t9_calib_get();

// Save/load calibration (EEPROM or flash)
bool t9_calib_save();
bool t9_calib_load(TouchCalibrationTable& out);

// ---------- Mapping API ----------

// Map filtered raw reading to screen coordinates using current calibration
// Returns false if no valid calibration loaded.
bool mapRawToScreen(const TCPoint& raw, TCPoint& screen);
