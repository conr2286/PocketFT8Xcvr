#pragma once

/**
 * @brief TouchPoint class (for conveying touchpad coordinates)
 */
class TouchPoint {
   public:
    TouchPoint(void) { x = y = z = 0; }  // Some TouchPad code depends upon zero initialization
    TouchPoint(unsigned x, unsigned y, unsigned z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    bool
    operator==(TouchPoint);
    bool operator!=(TouchPoint);

    int16_t x, y;  // Touchpoint screen coordinates
    int16_t z;     // true ==> valid coordinates, false ==> invalid coordinates (untouched)
};

/**
 * Driver for a 4-wire resistive touchpad
 *
 * @note Perhaps this should be a singleton but we're not in a caring mood today
 */
class TouchPad {
   public:
    TouchPad(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t xr, uint8_t yr);
    TouchPoint getTouchPoint(void);                       // Interrogates the touchpad device
    bool isNear(unsigned a, unsigned b, unsigned delta);  // Are two unsigned integers close in value?

   private:
    uint8_t xp, xm, yp, ym, xr, yr;  // The MCU pin numbers connected to touchpad's X+, X-, Y+ and Y- pins and their resistors

    void floatPin(unsigned pin);   // Disconnect pin from MCU digital circuitry
    void groundPin(unsigned pin);  // Ground (Logic 0) MCU pin
    void vccPin(unsigned pin);     // Drive MCU pin to Vcc (Logic 1)
};
