#pragma once

#include <Arduino.h>
#include <FS.h>
#include <HX8357_t3n.h>
#include "TouchPad.h"
#include "AGUI.h"

// Number of TouchScreen calibration target nodes
#define N_TARGETS 9

/**
 * @brief TouchScreeenPoint deals with points in the screen coordinate system
 */
class TouchScreenPoint {
   public:
    TouchScreenPoint(void) : x(0.0), y(0.0) {}

    TouchScreenPoint(const float cx, const float cy) : x(cx), y(cy) {}

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
    bool calibrate(void);
    unsigned getNTargets(void);
    TouchScreenPoint getTargetCoordinate(const unsigned idx);
    bool recordCalibrationNode(const unsigned idx, const TouchScreenPoint adc);
    bool doCalibration(AGUI& agui);
    bool isCalibrated(void) { return calibrated; }

    // Map filtered raw reading to screen coordinates using current calibration
    // Returns false if no valid calibration loaded.
    bool mapRawToScreen(const TouchPadPoint& raw, TouchScreenPoint& screen);
    void rotate(TouchScreenPoint& p);
    bool readTouchEvent(TouchScreenPoint& result);

    // Save/restore the calibration state to/from a stream
    bool serialize(File theFile);    // Save calibration data to a Stream
    bool deserialize(File theFile);  // Restore calibration data from a Stream

   private:
    // Our private methods and helpers
    TouchScreenPoint bilinear(const TouchCalibrationTable& cal, const TCZone& cell);
    bool locateCell(const TouchPadPoint& raw, TCZone& cell);
    const TouchCalibrationNode& nodeAt(const int r, const int cidx);
    uint16_t crc16(const uint8_t* data, const size_t length);
    void displayTarget(unsigned nodeIndex);
    void eraseTarget(unsigned nodeIndex);
    TouchScreenPoint toTCPoint(TouchPadPoint p);
    TouchScreenPoint readTouchPad(void);
    void waitForTouchEnd(void);

    // Attributes that should not be serialized
    TouchPad& touchPad;  // Reference to the touchpad driver
    HX8357_t3n& gfx;     // Reference to the touchscreen driver
    uint16_t width;      // TouchPad width in pixels
    uint16_t height;     // TouchPad height in pixels
    bool initialized;    // true if TouchScreen object has been initialized
    bool calibrated;     // true if TouchScreen object has calibration data

    // These calibration attributes must be serialized/deserialized to avoid recalibration
    TouchScreenPoint targetCoordinates[N_TARGETS];  // Screen coordinates of the calibration targets
    TouchCalibrationTable calibrationTable;         // This is the serializable calibration data
};
