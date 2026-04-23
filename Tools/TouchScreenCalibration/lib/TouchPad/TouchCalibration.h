#pragma once

class TouchCalibration {
   public:
    TouchCalibration(unsigned width, unsigned height, unsigned adcCardinality);
    bool setNode(unsigned ix, unsigned iy, unsigned screenX, unsigned screenY, unsigned adcX, unsigned adcY);

   private:
    // Record the display's width/height in pixels
    unsigned width;
    unsigned height;

    // Record the cardinality of the ADC results (e.g. a 10-bit ADC's cardinality is 1024)
    unsigned adcCardinality;

    // The Lagrange interpreter requires 9 calibration points known as nodes.  For AI convenience, we store
    // their x values separately from y values.  The stored values are raw ADC results from getTouchPoint().
    // On the touchpad surface, the upper-left node's XY results are xnodes[0],ynodes[0] and the lower-right
    // node's ADC results are xnodes[2],ynodes[2], representing the X and Y ADC results read at those
    // calibration targets (nodes).
    float xnodes[3];  // ADC values, e.g. {200.0, 1800.0, 3500.0};
    float ynodes[3];  // ADC values, e.g. {150.0, 1700.0, 3000.0};

    // Define the X and Y axis target screen coordinates for the 9 Lagrange nodes.
    // The screen coordinates of the 9 calibration targets used to acquire the
    // node (touchpad ADC results) values organize such that zx[0][0] is the screen
    // coordinate of the upper-left target, and zx[2][2] is the lower-right.
    float zx[3][3] = {
        {0.0, 160.0, 319.0},  // row for xnodes[0]
        {0.0, 160.0, 319.0},  // row for xnodes[1]
        {0.0, 160.0, 319.0}   // row for xnodes[2]
    };
    float zy[3][3] = {
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0}};
};