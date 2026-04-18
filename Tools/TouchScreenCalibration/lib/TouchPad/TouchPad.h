#pragma once

/**
 * @brief TouchEvent States
 */
enum TouchPointState {
    TS_NO_TOUCH = 0,  // Not touched (coordinates invalid)
    TS_TOUCH = 1,     // Touch (coordinates valid)
    TS_DRAG = 2       // Dragging (coordinates valid)
};

/**
 * @brief TouchPoint class (for conveying touchpad coordinates)
 */
class TouchPoint {
   public:
    TouchPoint(void) {
        x = y = 0;
        z = TS_NO_TOUCH;
    }  // Some TouchPad code depends upon zero initialization

    TouchPoint(unsigned x, unsigned y, TouchPointState z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    bool
    operator==(TouchPoint);
    bool operator!=(TouchPoint);

    int16_t x, y;       // Touchpoint screen coordinates
    TouchPointState z;  // Explains how to interpret coordinates
};

/**
 * Driver for a 4-wire resistive touchpad
 *
 * @note Perhaps this should be a singleton but we're not in a caring mood today
 */
class TouchPad {
   public:
    TouchPad(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t xr, uint8_t yr);
    TouchPoint getTouchPoint(void);                       // Low-level interrogation of touchpad
    TouchPoint getTouchEvent(void);                       // Higher-level analysis of touchpad event
    bool isNear(unsigned a, unsigned b, unsigned delta);  // Are two unsigned integers close in value?

   private:
    uint8_t xp, xm, yp, ym, xr, yr;  // The MCU pin numbers connected to touchpad's X+, X-, Y+ and Y- pins and their resistors
    TouchPointState state;           // TouchEvent's state variable
    uint16_t nCols, nRows;           // Touchscreen display's width and height in pixels
    uint16_t adcMax;                 // ADC returns values in the range 0..adcMax
    void floatPin(unsigned pin);     // Disconnect pin from MCU digital circuitry
    void groundPin(unsigned pin);    // Ground (Logic 0) MCU pin
    void vccPin(unsigned pin);       // Drive MCU pin to Vcc (Logic 1)
};
