/**
 * SYNOPSIS:
 *  Touch Calibrator corrects non-linearities and manufacturing variances in a resistive touchpad
 *
 * USAGE:
 *  An application executes a calibration procedure once for new touchpad hardware prior to
 *  its reliance on the touchpad.  The calibration procedure constructs a table of calibration
 *  data associating the touchpad's actual ADC readings with the display's screen coordinates.
 *
 * DISCUSSION:
 *  Calibration associates 9 on-screen target points with the touchpad's ADC readings for each.
 *  The 9 calibration points divide the touchscreen into four zones known as "cells" enabling
 *  bilinear interpolation within each cell.  Following calibration, the mapRawToScreen() method
 *  translates raw touchpad ADC coordinate readings into screen coordinates.
 *
 *  Without calibration, a touchpad reports touch events as raw ADC values without scaling
 *  to the display screen coordinate system, nor correcting for manufacturing misalignment
 *  between the physical touchpad and screen, margins, nor resistive nonlinearities arising
 *  near the corners.
 *
 *  This calibrator's implementation depends upon single-precision floating point calculations
 *  as the necessary hardware is commonly available on 32-bit MCUs supporting touchscreens.
 *
 *  This implementation relies upon a driver implemented in the TouchPad class to perform the
 *  raw touchpad ADC readings.
 *
 * ATTRIBUTION:
 *  Original implementation by Jim Conrad with significant guidance from CoPilot.
 *
 * LICENSE:
 *  Copyright (C) 2026 Jim Conrad
 *  MIT https://opensource.org/license/mit
 *
 */

#include "TouchCalibrator.h"
#include "hwdefs.h"
#include "TouchPad.h"
#include "DEBUG.h"

// We typically inset the calibration targets by a small margin to avoid
// asking operator to touch on the margin.
static const int TARGET_MARGIN = 15;  // Target offset in pixels from display edges

// Getter implementations
unsigned TouchCalibrator::getNTargets(void) { return N_TARGETS; }
TCPoint TouchCalibrator::getTargetCoordinate(unsigned idx) { return theTargetCoordinates[idx]; }

// Setter binds a target node's ADC readings to its screen coordinates in the calibration table
void TouchCalibrator::recordCalibrationNode(unsigned idx, TCPoint adc) {
    if (idx >= getNTargets()) return;                                  // Sanity check
    theCalibrationTable.nodes[idx].screen = getTargetCoordinate(idx);  // The target's screen coordinates
    theCalibrationTable.nodes[idx].raw = adc;                          // The touchpad's ADC coordinate values
    DPRINTF("Node[%d] maps touchpoint (%f,%f) to screen (%f,%f)\n", idx,
            theCalibrationTable.nodes[idx].raw.x, theCalibrationTable.nodes[idx].raw.y,
            theCalibrationTable.nodes[idx].screen.x, theCalibrationTable.nodes[idx].screen.y);
}

TouchCalibrator::TouchCalibrator(TouchPad& touchPadDriver) : touchPad(touchPadDriver) {}

void TouchCalibrator::setScreenSize(unsigned screenWidth, unsigned screenHeight) {
    // Calculate the target point margins and midpoints
    float margin = TARGET_MARGIN;
    float topMargin = margin;
    float botMargin = screenHeight - margin;
    float leftMargin = margin;
    float rightMargin = screenWidth - margin;
    unsigned horizMidpoint = screenWidth / 2;
    unsigned vertMidpoint = screenHeight / 2;

    // Initialize the screen coordinates of the calibration target points
    theTargetCoordinates[0] = TCPoint(leftMargin, topMargin);     // Upper-left target
    theTargetCoordinates[1] = TCPoint(horizMidpoint, topMargin);  // Top-center
    theTargetCoordinates[2] = TCPoint(rightMargin, topMargin);    // Upper-right
    theTargetCoordinates[3] = TCPoint(leftMargin, vertMidpoint);
    theTargetCoordinates[4] = TCPoint(horizMidpoint, vertMidpoint);  // Center
    theTargetCoordinates[5] = TCPoint(rightMargin, vertMidpoint);
    theTargetCoordinates[6] = TCPoint(leftMargin, botMargin);  // Lower-left
    theTargetCoordinates[7] = TCPoint(horizMidpoint, botMargin);
    theTargetCoordinates[8] = TCPoint(rightMargin, botMargin);  // Lower-right

}  // TouchCalibrator()

/**
 * @brief Find the calibration table node for row/col
 * @param c Calibration table
 * @param r Row index 0..2
 * @param cidx Column index 0..2
 * @return Calibration node for row/col
 */
const TouchCalibrationNode& TouchCalibrator::nodeAt(const TouchCalibrationTable& c, int r, int cidx) {
    // Sanity checks
    if ((r < 0) || (r >= 3) || (cidx < 0) || (cidx >= 3)) {
        DTRACE();
    }
    return c.nodes[r * 3 + cidx];
}

/**
 * @brief Helper returns the maximum of three values
 * @param a value1
 * @param b value2
 * @param c value3
 * @return maximum value
 */
static float max(float a, float b, float c) {
    if (a > b && a > c) return a;
    if (b > a && b > c) return b;
    return c;
}

/**
 * @brief Locate which of the four rectangular cells the touchpoint resides
 * @param cal Calibration table
 * @param raw Touchpoint x/y coordinates
 * @param cell Returned cell
 * @return Error indication
 *
 * @note The interpolator divides the touchscreen into four rectangular zones known as cells
 * and performs a bilinear interpolation within the cell containing the touchpoint.
 *
 * @note The calibration procedure's touchpoints likely did not exactly lie on the screen (and
 * cell) edges.  When asked to interpolate a raw touchpoint lying outside the known cellular
 * region, we snap to the nearby cell's row/col.
 */
bool TouchCalibrator::locateCell(const TouchCalibrationTable& cal, const TCPoint& raw, TCZone& cell) {
    int row = -1, col = -1;

    // Perhaps the raw touchpoint lies above the top row
    float top = max(cal.nodes[0].raw.y, cal.nodes[1].raw.y, cal.nodes[2].raw.y);
    if (raw.y < top) {
        row = 0;  // Snap to the top row
        DTRACE();
    } else {
        // Find the row band containing the touchpoint
        for (int r = 0; r < 2; r++) {
            float y0 = nodeAt(cal, r, 0).raw.y;
            float y1 = nodeAt(cal, r + 1, 0).raw.y;
            if ((raw.y >= y0 && raw.y <= y1) || (raw.y <= y0 && raw.y >= y1)) {
                DPRINTF("raw.y=%f y0=%f y1=%f\n", raw.y, y0, y1);
                row = r;
                break;
            }
        }
    }

    // If we haven't yet found the row, then it must lie below the bottom boundary
    if (row == -1) {
        row = 1;  // Snap to the bottom row
        DTRACE();
    }

    // Perhaps the raw touchpoint lies to the left of the leftmost column
    float leftmost = max(cal.nodes[0].raw.x, cal.nodes[3].raw.x, cal.nodes[6].raw.x);
    if (raw.x < leftmost) {
        col = 0;  // Snap to leftmost column
        DTRACE();
    } else {
        // Find column band
        for (int c = 0; c < 2; c++) {
            float x0 = nodeAt(cal, 0, c).raw.x;
            float x1 = nodeAt(cal, 0, c + 1).raw.x;
            if ((raw.x >= x0 && raw.x <= x1) || (raw.x <= x0 && raw.x >= x1)) {
                col = c;
                break;
            }
        }
    }

    // If we haven't found the column, it must lie beyond the rightmost boundary
    if (col == -1) {
        col = 1;
        DTRACE();
    }

    DPRINTF("row=%d, col=%d\n", row, col);
    // if ((row < 0) || (col < 0)) return false;

    const TouchCalibrationNode& n00 = nodeAt(cal, row, col);
    const TouchCalibrationNode& n10 = nodeAt(cal, row, col + 1);
    const TouchCalibrationNode& n01 = nodeAt(cal, row + 1, col);

    float dx = n10.raw.x - n00.raw.x;
    float dy = n01.raw.y - n00.raw.y;
    if (dx == 0.0f || dy == 0.0f)
        return false;

    float u = (raw.x - n00.raw.x) / dx;
    float v = (raw.y - n00.raw.y) / dy;

    cell.row = row;
    cell.col = col;
    cell.u = u;
    cell.v = v;
    return true;
}

/**
 * @brief Bilinear interpolator
 * @param cal Calibration table
 * @param cell Bilinear cell containing the touchpoint
 * @return Screen coordinates of interpolated point
 *
 * locateCell() selected a cell bounded by nodes (n00,n10,n01,n11) and calculated
 * the normalized 0..1 offset (u,v) of the touchpoint within our cell parameter.
 *
 *
 *    v
 *    |
 *    |   (u=0,v=0) n00 ----------- n10 (u=1,v=0)
 *    |              |               |
 *    |              |               |
 *    |              |               |
 *    |   (u=0,v=1) n01 ----------- n11 (u=1,v=1)
 *    |
 *    +----------------------------------------→ u
 *
 *
 */
TCPoint TouchCalibrator::bilinear(const TouchCalibrationTable& cal, const TCZone& cell) {
    // Identify the cell's four corners
    const TouchCalibrationNode& n00 = nodeAt(cal, cell.row, cell.col);
    const TouchCalibrationNode& n10 = nodeAt(cal, cell.row, cell.col + 1);
    const TouchCalibrationNode& n01 = nodeAt(cal, cell.row + 1, cell.col);
    const TouchCalibrationNode& n11 = nodeAt(cal, cell.row + 1, cell.col + 1);

    float u = cell.u;  // Normalized x placement 0..1 of raw touch within cell
    float v = cell.v;  // Normalized y placement 0..1 of raw touch within cell

    DPRINTF("u=%f v=%f\n", u, v);

    TCPoint out;

    // For x-Axis, blend weights within that cell.  If the touch was in this cell's exact
    // center, the weights would be u==v==0.5 as each corner gets weight 0.25
    out.x =
        (1 - u) * (1 - v) * n00.screen.x +  // Weight of top-left
        (u) * (1 - v) * n10.screen.x +      // Weight of top-right
        (1 - u) * (v)*n01.screen.x +        // Weight of bottom-left
        (u) * (v)*n11.screen.x;             // Weight of bottom-right

    // Repeat for y-Axis
    out.y =
        (1 - u) * (1 - v) * n00.screen.y +
        (u) * (1 - v) * n10.screen.y +
        (1 - u) * (v)*n01.screen.y +
        (u) * (v)*n11.screen.y;

    return out;
}

bool TouchCalibrator::mapRawToScreen(const TCPoint& raw, TCPoint& screen) {
    TCZone cell;
    if (!locateCell(theCalibrationTable, raw, cell)) {
        DPRINTF("locateCell raw.x=%f raw.y=%f\n failed\n", raw.x, raw.y);
        return false;
    }

    screen = bilinear(theCalibrationTable, cell);

    // Deal with inaccuracies near edges
    if (screen.x < 0) screen.x = 0;
    if (screen.x > 319) screen.x = 319;
    if (screen.y < 0) screen.y = 0;
    if (screen.y >= 479) screen.y = 479;

    return true;
}
