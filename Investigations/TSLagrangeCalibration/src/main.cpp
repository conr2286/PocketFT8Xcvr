#include <Arduino.h>

#include "Lagrange.cpp"

int main(void) {
    Serial.begin(9600);
    Serial.printf("Starting...\n");

    // ------------------------------------------------------------
    // Calibration grid (example)
    // Raw ADC X and Y nodes (monotonic)
    // ------------------------------------------------------------
    // double xnodes[3] = {200.0, 1800.0, 3500.0};
    // double ynodes[3] = {150.0, 1700.0, 3300.0};
    double xnodes[3] = {200.0, 1800.0, 3500.0};
    double ynodes[3] = {150.0, 1700.0, 3300.0};

    // ------------------------------------------------------------
    // Z-values: physical screen coordinates (example)
    // z[i][j] corresponds to (xnodes[i], ynodes[j])
    //
    // For a touch panel, you typically calibrate X and Y separately.
    // Here we show the X-surface; repeat for Y-surface.
    // ------------------------------------------------------------
    double zx[3][3] = {
        {0.0, 160.0, 319.0},  // row for xnodes[0]
        {0.0, 160.0, 319.0},  // row for xnodes[1]
        {0.0, 160.0, 319.0}   // row for xnodes[2]
    };

    // Same idea for Y mapping:
    double zy[3][3] = {
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0}};

    double rawX, rawY;
    double physX, physY;

    // Interpolate upper left
    rawX = 200.0;
    rawY = 150.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolate at (%f,%f) is (%f, %f)\n", rawX, rawY, physX, physY);

    // Interpolate upper right
    rawX = 3500.0;
    rawY = 150.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolate at (%f,%f) is (%f, %f)\n", rawX, rawY, physX, physY);

    // Interpolate
    rawX = 2100.0;
    rawY = 1900.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolate at (%f,%f) is (%f, %f)\n", rawX, rawY, physX, physY);

    // Interpolate
    rawX = 2100.0;
    rawY = 1900.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolate at (%f,%f) is (%f, %f)\n", rawX, rawY, physX, physY);

    return 0;
}
