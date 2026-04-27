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

// Record the calibration table as a linear array of bindings between ADC and screen coordinates
struct TouchCalibrationTable {
    TouchCalibrationNode nodes[9];  // row-major 3x3
};

// Getters and setters
unsigned getNTargets(void);
TCPoint getTargetCoordinate(unsigned idx);
void recordCalibrationNode(unsigned idx, TCPoint adc);

// Get final calibration (valid when state == Done)
const TouchCalibrationTable& t9_calib_get();

// Map filtered raw reading to screen coordinates using current calibration
// Returns false if no valid calibration loaded.
bool mapRawToScreen(const TCPoint& raw, TCPoint& screen);
