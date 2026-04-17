#pragma once

#include <Arduino.h>
#include "hwdefs.h"

// ---------- Basic types ----------

struct T9Point {
    float x;
    float y;
};

struct T9CalibNode {
    T9Point raw;     // raw ADC reading
    T9Point screen;  // ideal screen coordinate
};

struct T9Calib9 {
    T9CalibNode nodes[9];  // row-major 3x3
};

// ---------- Public configuration ----------

// Adjust these to your wiring
namespace T9Pins {
// Resistive touch pins
constexpr int XP = PIN_XP;  // example
constexpr int XM = PIN_XM;
constexpr int YP = PIN_YP;
constexpr int YM = PIN_YM;
}  // namespace T9Pins

// Screen resolution
constexpr int T9_SCREEN_WIDTH = 320;
constexpr int T9_SCREEN_HEIGHT = 480;

// ---------- Driver API ----------

void t9_init();

// Returns true if a touch is detected and fills raw (0..1023)
bool t9_read_raw(T9Point& raw, uint16_t& z);

// Filtered raw (median/oversampled). Returns true if touch present.
bool t9_read_filtered(T9Point& raw, uint16_t& z);

// ---------- Calibration API ----------

enum class T9CalState {
    Idle,
    Running,
    Done
};

// Callback type: draw target i (0..8) at given screen coordinate
using T9DrawTargetFn = void (*)(int index, const T9Point& screenPos);

// Start 9-point calibration
void t9_calib_start(T9DrawTargetFn drawFn);

// Call periodically (e.g. in loop)
void t9_calib_update();

// Get calibration state
T9CalState t9_calib_state();

// Get final calibration (valid when state == Done)
const T9Calib9& t9_calib_get();

// Save/load calibration (EEPROM or flash)
bool t9_calib_save();
bool t9_calib_load(T9Calib9& out);

// ---------- Mapping API ----------

// Map filtered raw reading to screen coordinates using current calibration
// Returns false if no valid calibration loaded.
bool t9_map_raw_to_screen(const T9Calib9& cal, const T9Point& raw, T9Point& screen);
