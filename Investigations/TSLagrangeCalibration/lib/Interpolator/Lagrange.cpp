#include "Lagrange.h"

// ============================================================
// 1D quadratic Lagrange basis for nodes x0, x1, x2
// ============================================================
static void lagrange3(double x,
                      double x0, double x1, double x2,
                      double L[3]) {
    L[0] = (x - x1) * (x - x2) / ((x0 - x1) * (x0 - x2));
    L[1] = (x - x0) * (x - x2) / ((x1 - x0) * (x1 - x2));
    L[2] = (x - x0) * (x - x1) / ((x2 - x0) * (x2 - x1));
}

// ============================================================
// 2D tensor-product quadratic interpolation
// ============================================================
double interp2D_tensor_quad(double x, double y,
                            double xnodes[3],
                            double ynodes[3],
                            double z[3][3]) {
    double Lx[3], Ly[3];

    lagrange3(x, xnodes[0], xnodes[1], xnodes[2], Lx);
    lagrange3(y, ynodes[0], ynodes[1], ynodes[2], Ly);

    double sum = 0.0;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sum += z[i][j] * Lx[i] * Ly[j];

    return sum;
}
