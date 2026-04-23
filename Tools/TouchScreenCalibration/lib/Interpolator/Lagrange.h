#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// 2D tensor-product quadratic interpolation using Lagrange basis
// ============================================================

/**
 * Performs 2D tensor-product quadratic interpolation using Lagrange polynomials
 *
 * @param x         X coordinate to interpolate at
 * @param y         Y coordinate to interpolate at
 * @param xnodes    Array of 3 X node coordinates (must be monotonic)
 * @param ynodes    Array of 3 Y node coordinates (must be monotonic)
 * @param z         3x3 array of Z values at grid points z[i][j] = f(xnodes[i], ynodes[j])
 * @return          Interpolated value at (x,y)
 */
float interp2D_tensor_quad(float x, float y,
                           float xnodes[3],
                           float ynodes[3],
                           float z[3][3]);

#ifdef __cplusplus
}
#endif
