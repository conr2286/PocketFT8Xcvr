#pragma once

/**
 * @brief TouchPadPoint class conveys raw touchpad coordinates as raw ADC values
 */
class TouchPadPoint {
   public:
    TouchPadPoint(void) {
        x = y = 0;
    }  // Some TouchPad code depends upon zero initialization

    TouchPadPoint(unsigned x, unsigned y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(TouchPadPoint);
    bool operator!=(TouchPadPoint);

    int16_t x, y;  // ADC results, not screen coordinates
};

/**
 * Driver for a 4-wire resistive touchpad
 *
 * DISCUSSION:
 *  + TouchPad class works with the touchpad's raw ADC readings, unscaled to the
 *  screen coordinate system, uncorrected for misalignment between the touchpad and
 *  the display screen, margins, rotation, or resistive non-linearities.
 *  + Perhaps this should be a singleton but weren't in a caring mood today
 */
class TouchPad {
   public:
    TouchPad(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t xr, uint8_t yr);
    bool readRaw(TouchPadPoint& result);                  // Non-blocking, low-level, interrogation of touchpad ADC coordinates
    bool readFiltered(TouchPadPoint& result);             // Non-blocking, stateful, filtered interrogation of touchpad ADC coordinates
    bool isNear(unsigned a, unsigned b, unsigned delta);  // Are two unsigned integers close in value?

   private:
    uint8_t xp, xm, yp, ym, xr, yr;  // The MCU pin numbers connected to touchpad's X+, X-, Y+ and Y- pins and their resistors
    uint16_t adcMax;                 // ADC returns values in the range 0..adcMax
    void floatPin(unsigned pin);     // Disconnect pin from MCU digital circuitry
    void groundPin(unsigned pin);    // Ground (Logic 0) MCU pin
    void vccPin(unsigned pin);       // Drive MCU pin to Vcc (Logic 1)
    template <int N>
    void sort_small(uint16_t (&a)[N]);
};
