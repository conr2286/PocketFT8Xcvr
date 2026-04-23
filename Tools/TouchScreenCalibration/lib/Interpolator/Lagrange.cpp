

// ============================================================
// 1D quadratic Lagrange basis for nodes x0, x1, x2
// ============================================================
static inline void lagrange3(float x,
                             float x0, float x1, float x2,
                             float L[3]) {
    L[0] = (x - x1) * (x - x2) / ((x0 - x1) * (x0 - x2));
    L[1] = (x - x0) * (x - x2) / ((x1 - x0) * (x1 - x2));
    L[2] = (x - x0) * (x - x1) / ((x2 - x0) * (x2 - x1));
}  // lagrange3()

// ============================================================
// 2D tensor-product quadratic interpolation
// ============================================================
float interp2D_tensor_quad(float x, float y,
                           float xnodes[3],
                           float ynodes[3],
                           float z[3][3]) {
    float Lx[3], Ly[3];

    lagrange3(x, xnodes[0], xnodes[1], xnodes[2], Lx);
    lagrange3(y, ynodes[0], ynodes[1], ynodes[2], Ly);

    float sum = 0.0;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sum += z[i][j] * Lx[i] * Ly[j];

    return sum;
}  // interp2D_tensor_quad()

// =============
== == == == == == == == == == == == == == == == == == == == == == == =
                                                                         // Example: 9-node calibration + interpolation
                                                                         // ============================================================
    int main(void) {
    // ------------------------------------------------------------
    // Calibration grid (example)
    // Raw ADC X and Y nodes (monotonic)
    // ------------------------------------------------------------
    float xnodes[3] = {200.0, 1800.0, 3500.0};
    // float ynodes[3] = {150.0, 1700.0, 3300.0};
    float ynodes[3] = {150.0, 1700.0, 3000.0};

    // ------------------------------------------------------------
    // Z-values: physical screen coordinates (example)
    // z[i][j] corresponds to (xnodes[i], ynodes[j])
    //
    // For a touch panel, you typically calibrate X and Y separately.
    // Here we show the X-surface; repeat for Y-surface.
    // ------------------------------------------------------------
    float zx[3][3] = {
        {0.0, 160.0, 319.0},  // row for xnodes[0]
        {0.0, 160.0, 319.0},  // row for xnodes[1]
        {0.0, 160.0, 319.0}   // row for xnodes[2]
    };

    // Same idea for Y mapping:
    float zy[3][3] = {
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0},
        {0.0, 120.0, 239.0}};

    // Interpolate 2100,1900
    float rawX = 2100.0;
    float rawY = 1900.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    float physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    float physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolated physical point = (%f, %f)\n", physX, physY);

    // Interpolate 100,100
    rawX = 100.0;
    rawY = 100.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolated physical point = (%f, %f)\n", physX, physY);

    // Interpolate
    rawX = 200.0;
    rawY = 150.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolated physical point = (%f, %f)\n", physX, physY);

    // Interpolate
    rawX = 3500.0;
    rawY = 3000.0;

    // ------------------------------------------------------------
    // Interpolate X and Y physical coordinates
    // ------------------------------------------------------------
    physX = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zx);
    physY = interp2D_tensor_quad(rawX, rawY, xnodes, ynodes, zy);

    printf("Interpolated physical point = (%f, %f)\n", physX, physY);

    return 0;
}
