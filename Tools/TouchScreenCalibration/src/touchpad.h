#pragma once

// Touchpoint coordinates
class TouchPoint {
   public:
    TouchPoint(void) { x = y = z = 0; }

    bool operator==(TouchPoint);
    bool operator!=(TouchPoint);

    int16_t x, y, z;  // Touchpoint screen (*not* ADC) coordinates
};

TouchPoint getTouchPoint(void);