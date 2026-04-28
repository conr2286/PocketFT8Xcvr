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
 * @brief TouchPadPoint class (for conveying raw touchpad coordinates as ADC results)
 */
class TouchPadPoint {
   public:
    TouchPadPoint(void) {
        x = y = 0;
        state = TS_NO_TOUCH;
    }  // Some TouchPad code depends upon zero initialization

    TouchPadPoint(unsigned x, unsigned y, TouchPointState z) {
        this->x = x;
        this->y = y;
        this->state = z;
    }

    bool operator==(TouchPadPoint);
    bool operator!=(TouchPadPoint);

    int16_t x, y;           // Touchpoint screen coordinates
    TouchPointState state;  // Explains how to interpret coordinates
};

/**
 * Driver for a 4-wire resistive touchpad
 *
 * DISCUSSION:
 *  + The TouchPad class works with the touchpad's raw ADC readings, unscaled to the
 *  screen coordinate system, uncorrected for misalignment between the touchpad and
 *  the display screen, margins, or resistive non-linearities.
 *  + Perhaps this should be a singleton but we're not in a caring mood today
 */
class TouchPad {
   public:
    TouchPad(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t xr, uint8_t yr);
    TouchPadPoint getTouchPoint(void);                    // Non-blocking, low-level, interrogation of touchpad ADC coordinates
    TouchPadPoint getTouchEvent(void);                    // Non-blocking, stateful, filtered interrogation of touchpad ADC coordinates
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
