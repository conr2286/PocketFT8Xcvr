#pragma once

#include <Arduino.h>
#include "TouchPad.h"

// ---------- Basic types ----------

class TCPoint {
   public:
    TCPoint(void) { x = y = 0.0; }

    TCPoint(const float x, const float y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(TCPoint);
    bool operator!=(TCPoint);

    float x;
    float y;
};

// We divide the touchpad into four TouchCalibrator Zones and bilinear interpolate within each
struct TCZone {
    int row;  // This zone's row 0..1
    int col;  // This zone's col 0..1
    float u;
    float v;
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

#define N_TARGETS 9

class TouchCalibrator {
   public:
    // Constructors
    TouchCalibrator(TouchPad& touchPadDriver, unsigned screenWidth, unsigned screenHeight);

    // Getters and setters
    unsigned getNTargets(void);
    TCPoint getTargetCoordinate(unsigned idx);
    void recordCalibrationNode(unsigned idx, TCPoint adc);

    // Get final calibration (valid when state == Done)
    const TouchCalibrationTable& t9_calib_get();

    // Map filtered raw reading to screen coordinates using current calibration
    // Returns false if no valid calibration loaded.
    bool mapRawToScreen(const TCPoint& raw, TCPoint& screen);

   private:
    TCPoint bilinear(const TouchCalibrationTable& cal, const TCZone& cell);
    bool locateCell(const TouchCalibrationTable& cal, const TCPoint& raw, TCZone& cell);
    const TouchCalibrationNode& nodeAt(const TouchCalibrationTable& c, int r, int cidx);

    TCPoint theTargetCoordinates[N_TARGETS];
    TouchPad& touchPad;
};
