

#include <Arduino.h>

#include "TouchCalibration.h"

/**
 * SYNOPSIS
 *  A Calibrated Interface to a Resistive Touchpad
 *
 * USAGE
 *
 * DISCUSSION
 *  This "calibrated" interface yields touchpad XY results in the screen
 *  coordinate system as opposed to raw ADC results.
 *
 *  Calibration is not a simple scaling of the raw ADC data:
 *      + Touchpads are not perfectly aligned (glued) to the display screen
 *      + Resistive touchpads return non-linear results
 *      + Their actual X and Y axis resistances vary from part to part
 *
 * DESIGN
 *  Calibration associates the XY screen coordinates of 9 targets (known as "nodes")
 *  with their raw XY ADC results from the resistive touchpad.  We use these attributes
 *  to construct coefficients for a 2 dimension tensor-product quadratic interpolator.
 *  The interpolation function later transforms raw ADC coordinates acquired from the
 *  touchpad into screen coordinates.
 */

/**
 * @brief Bind a touchpad calibrator to the specified screen parameters
 * @param width The display screen's width in pixels
 * @param height The display screen's height in pixels
 * @param adcCardinality The size of the set of possible ADC values (e.g. 1024)
 *
 * @note The width and height refer to the display's X and Y axis as defined by
 * the hardware (not by however you may have rotated the display).
 */
TouchCalibration::TouchCalibration(unsigned width, unsigned height, unsigned adcCardinality) {
    this->width = width;
    this->height = height;
    this->adcCardinality = adcCardinality;
}  // TouchCalibration()

/**
 * @brief Record the attributes of a touchpad calibration target node
 * @param ix The target node's x index, 0..2
 * @param iy The target node's y index, 0..2
 * @param screenX This target node's X screen coordinate
 * @param screenY This target node's Y screen coordinate
 * @param adcX This target node's X ADC reading
 * @param adcY This target node's Y ADC reading
 * @return Error indication (0==>Ok, 1==>Error)
 *
 * @note The target node's indices identify one of the nine Lagrange calibration nodes; they
 * are not coordinates.  Node [0][0] refers to the upper-left while [2][2] the lower-right target.
 */
bool TouchCalibration::setNode(unsigned ix, unsigned iy, unsigned screenX, unsigned screenY, unsigned adcX, unsigned adcY) {
    // Sanity checks
    if (ix >= 3 || iy >= 3) return true;                                // Invalid target indices
    if (screenX >= width || screenY >= height) return true;             // Invalid target screen coordinates
    if (adcX >= adcCardinality || adcY >= adcCardinality) return true;  // Invalid ADC values

    // Record the target node's screen coordinates
    zx[ix][iy] = screenX;  // Target node's X screen coordinate
    zy[ix][iy] = screenY;  // Target node's Y screen coordinate

    // Bind the touchpad's raw ADC values to these screen coordinates forming a calibration node for the Lagrange interpreter
    xnodes[ix] = adcX;
    ynodes[iy] = adcY;
}
