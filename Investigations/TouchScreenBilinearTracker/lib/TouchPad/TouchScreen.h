#pragma once

#include <Arduino.h>
#include <FS.h>
#include <HX8357_t3n.h>
#include "TouchPad.h"

// ---------- Basic types ----------

#define N_TARGETS 9

class TouchScreenPoint {
   public:
    TouchScreenPoint(void) { x = y = 0.0; }

    TouchScreenPoint(const float x, const float y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(TouchScreenPoint);
    bool operator!=(TouchScreenPoint);

    float x;
    float y;
};

// We divide the touchpad into four TouchScreen Zones and bilinear interpolate within each
struct TCZone {
    int row;  // This zone's row 0..1
    int col;  // This zone's col 0..1
    float u;
    float v;
};

// The touchpad is calibrated at 9 on-screen targets known as "nodes" each
// associating a raw ADC coordinate tuple with a screen coordinate tuple.
struct TouchCalibrationNode {
    TouchScreenPoint raw;     // raw ADC reading
    TouchScreenPoint screen;  // ideal screen coordinate
};

// Record the calibration table as a linear array of bindings between ADC and screen coordinates
struct TouchCalibrationTable {
    TouchCalibrationNode nodes[9];  // row-major 3x3 calibration data
    uint16_t checksum;              // Node data checksum (used in serialization)
};

class TouchScreen {
   public:
    // Constructor
    TouchScreen(TouchPad& touchPadDriver, HX8357_t3n& touchScreenDriver);

    // Methods associated with performing a calibration of the touchpad to the screen
    void begin(void);
    unsigned getNTargets(void);
    TouchScreenPoint getTargetCoordinate(const unsigned idx);
    bool recordCalibrationNode(const unsigned idx, const TouchScreenPoint adc);

    // Map filtered raw reading to screen coordinates using current calibration
    // Returns false if no valid calibration loaded.
    bool mapRawToScreen(const TouchScreenPoint& raw, TouchScreenPoint& screen);
    void rotate(TouchScreenPoint& p);

    // Save/restore the calibration state to/from a stream
    bool serialize(File theFile);    // Save calibration data to a Stream
    bool deserialize(File theFile);  // Restore calibration data from a Stream

   private:
    // Our private methods and helpers
    TouchScreenPoint bilinear(const TouchCalibrationTable& cal, const TCZone& cell);
    bool locateCell(const TouchScreenPoint& raw, TCZone& cell);
    const TouchCalibrationNode& nodeAt(const int r, const int cidx);
    uint16_t crc16(const uint8_t* data, const size_t length);

    // Attributes that need not be serialized
    TouchPad& touchPad;  // The touchpad driver
    HX8357_t3n& gfx;     // The touchscreen driver

    // These calibration attributes must be serialized/deserialized to avoid recalibration
    TouchScreenPoint targetCoordinates[N_TARGETS];  // Screen coordinates of the calibration targets
    TouchCalibrationTable theCalibrationTable;      // This is the serializable calibration data
};
