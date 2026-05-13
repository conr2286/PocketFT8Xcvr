/**
 * SYNOPSIS:
 *  TouchScreen interrogates the touch pad returning touchscreen coordinates
 *
 * USAGE:
 *  An application must execute a calibration procedure once for the touchpad.
 *  The calibration procedure constructs a table of calibration "nodes" that map
 *  the touchpad's raw ADC readings with the display's screen coordinates.
 *
 * DISCUSSION:
 *  We oft use the term TouchScreen to refer to the assembly of the touch pad glued to the
 *  display screen, and the term TouchPad to refer to the touch pad hardware independent
 *  of the display.   The TouchScreen class is responsible for calibration and mapping
 *  TouchPad coordinates into TouchScreen coordinates.
 *
 *  Calibration associates 9 on-screen target points with the touchpad's ADC readings for each.
 *  The 9 calibration points divide the touchscreen into four zones known as "cells" enabling
 *  bilinear interpolation within each cell.  Following calibration, the mapRawToScreen() method
 *  translates raw touchpad ADC coordinate readings into screen coordinates.
 *
 *  Without calibration, a touchpad reports coordinates as raw ADC values without scaling
 *  to the display screen coordinate system, correcting for manufacturing misalignment
 *  between the physical touchpad and screen, margins, nor resistive nonlinearities arising
 *  near the corners.
 *
 *  This calibrator's implementation depends upon single-precision floating point calculations
 *  as the necessary hardware is commonly available on 32-bit MCUs supporting touchscreens.
 *
 *  TouchScreen relies upon the TouchPad driver (class) to perform the raw ADC readings.
 *
 *  The TouchScreen class declares methods for serializing/deserializing the calibration data
 *  but these are implemented in TouchSerialization.cpp, not here.
 *
 * ATTRIBUTION:
 *  Original implementation by Jim Conrad with crucial guidance from CoPilot.  Say what you want,
 *  but the damn thing was more useful than some humans I've worked with over the past 50 years.
 *
 * LICENSE:
 *  Copyright (C) 2026 Jim Conrad
 *  MIT License at https://opensource.org/license/mit
 *
 */

#include <Arduino.h>
#include "HX8357_t3n.h"
#include "TouchScreen.h"
#include "hwdefs.h"
#include "TouchPad.h"
#include "NODEBUG.h"

// We typically inset the calibration targets by a small margin to avoid
// asking operator to touch on the margin.
static const int TARGET_MARGIN = 30;  // Target offset in pixels from display edges

/**
 * @brief Get the number of calibration targets used by the interpolator
 * @return Count of targets
 */
unsigned TouchScreen::getNTargets(void) { return N_TARGETS; }

/**
 * @brief Get the target screen coordinates for the specified node
 * @param idx Node index
 * @return The X/Y coordinates
 */
TouchScreenPoint TouchScreen::getTargetCoordinate(unsigned idx) { return targetCoordinates[idx]; }

/**
 * @brief Record a single node in the calibration table
 * @param idx Selects which node
 * @param adc TouchPad's ADC X/Y coordinates for this node
 * @return true if successful
 *
 * WARNING:
 *  Calibration must be performed with GFX in its default rotation (i.e. setRotation(0))
 *
 * DISCUSSION:
 *  The calibration table contains a "node" entry for each calibration target.  The table
 *  associates TouchPad ADC X/Y coordinates with display screen coordinates.  The bilinear
 *  interpolator uses this table to map arbitrary TouchPad coordinate to TouchScreen coordinates.
 */
bool TouchScreen::recordCalibrationNode(unsigned idx, TouchScreenPoint adc) {
    // Sanity checks
    if (idx >= getNTargets()) return false;                         // Ridiculous index
    if (gfx.getRotation() != 0) return false;                       // Calibration must occur unrotated
    calibrationTable.nodes[idx].screen = getTargetCoordinate(idx);  // The target's screen coordinates
    calibrationTable.nodes[idx].raw = adc;                          // The touchpad's ADC coordinate values
    DPRINTF("Node[%d] maps touchpoint (%f,%f) to screen (%f,%f)\n", idx,
            calibrationTable.nodes[idx].raw.x, calibrationTable.nodes[idx].raw.y,
            calibrationTable.nodes[idx].screen.x, calibrationTable.nodes[idx].screen.y);
    return true;
}

/**
 * @brief Constructor
 * @param touchPadDriver Reference to the TouchPad driver
 * @param gfxDriver Reference to the Adafruit GFX driver
 */
TouchScreen::TouchScreen(TouchPad& touchPadDriver, HX8357_t3n& gfxDriver) : touchPad(touchPadDriver), gfx(gfxDriver) {
}

/**
 * @brief Initialize the TouchScreen object
 *
 * DISCUSSION:
 *  We initialize here rather than in constructor to avoid invoking methods in potentially uninitialized
 *  objects (e.g. gfx) referenced here.
 */
void TouchScreen::begin() {
    // The Adafruit GFX driver knows the touchpad's width and height (in pixels)
    uint8_t gfxRotation = gfx.getRotation();  // The GFX driver's screen rotation code
    gfx.setRotation(0);                       // We need the touchpad's width/height in its natural rotation
    width = gfx.width();                      // Record touchpad hardware width (pixels) from GFX
    height = gfx.height();                    // And height in pixels
    gfx.setRotation(gfxRotation);             // Restore application's GFX rotation

    // Calculate the target point margins and midpoints
    float margin = TARGET_MARGIN;
    float topMargin = margin;
    float botMargin = height - margin;
    float leftMargin = margin;
    float rightMargin = width - margin;
    unsigned horizMidpoint = width / 2;
    unsigned vertMidpoint = height / 2;
    DPRINTF("leftMargin=%f rightMargin=%f topMargin=%f botMargin=%f TARGET_MARGIN=%d\n", leftMargin, rightMargin, topMargin, botMargin, TARGET_MARGIN);

    // Initialize the screen coordinates of the calibration target points
    targetCoordinates[0] = TouchScreenPoint(leftMargin, topMargin);     // Upper-left target
    targetCoordinates[1] = TouchScreenPoint(horizMidpoint, topMargin);  // Top-center
    targetCoordinates[2] = TouchScreenPoint(rightMargin, topMargin);    // Upper-right
    targetCoordinates[3] = TouchScreenPoint(leftMargin, vertMidpoint);
    targetCoordinates[4] = TouchScreenPoint(horizMidpoint, vertMidpoint);  // Center
    targetCoordinates[5] = TouchScreenPoint(rightMargin, vertMidpoint);
    targetCoordinates[6] = TouchScreenPoint(leftMargin, botMargin);  // Lower-left
    targetCoordinates[7] = TouchScreenPoint(horizMidpoint, botMargin);
    targetCoordinates[8] = TouchScreenPoint(rightMargin, botMargin);  // Lower-right

}  // TouchScreen()

/**
 * @brief Find the calibration table node for row/col
 * @param c Calibration table
 * @param r Row index 0..2
 * @param cidx Column index 0..2
 * @return Calibration node for row/col
 */
const TouchCalibrationNode& TouchScreen::nodeAt(int r, int cidx) {
    // Sanity checks
    if ((r < 0) || (r >= 3) || (cidx < 0) || (cidx >= 3)) {
        DTRACE();
    }
    return calibrationTable.nodes[r * 3 + cidx];
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
bool TouchScreen::locateCell(const TouchPadPoint& raw, TCZone& cell) {
    int row = -1, col = -1;

    // Perhaps the raw touchpoint lies above the top row
    float top = max(calibrationTable.nodes[0].raw.y, calibrationTable.nodes[1].raw.y, calibrationTable.nodes[2].raw.y);
    if (raw.y < top) {
        row = 0;  // Snap to the top row
        DTRACE();
    } else {
        // Find the row band containing the touchpoint
        for (int r = 0; r < 2; r++) {
            float y0 = nodeAt(r, 0).raw.y;
            float y1 = nodeAt(r + 1, 0).raw.y;
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
    float leftmost = max(calibrationTable.nodes[0].raw.x, calibrationTable.nodes[3].raw.x, calibrationTable.nodes[6].raw.x);
    if (raw.x < leftmost) {
        col = 0;  // Snap to leftmost column
        DTRACE();
    } else {
        // Find column band
        for (int c = 0; c < 2; c++) {
            float x0 = nodeAt(0, c).raw.x;
            float x1 = nodeAt(0, c + 1).raw.x;
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

    const TouchCalibrationNode& n00 = nodeAt(row, col);
    const TouchCalibrationNode& n10 = nodeAt(row, col + 1);
    const TouchCalibrationNode& n01 = nodeAt(row + 1, col);

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
TouchScreenPoint TouchScreen::bilinear(const TouchCalibrationTable& cal, const TCZone& cell) {
    // Identify the cell's four corners
    const TouchCalibrationNode& n00 = nodeAt(cell.row, cell.col);
    const TouchCalibrationNode& n10 = nodeAt(cell.row, cell.col + 1);
    const TouchCalibrationNode& n01 = nodeAt(cell.row + 1, cell.col);
    const TouchCalibrationNode& n11 = nodeAt(cell.row + 1, cell.col + 1);

    float u = cell.u;  // Normalized x placement 0..1 of raw touch within cell
    float v = cell.v;  // Normalized y placement 0..1 of raw touch within cell

    DPRINTF("u=%f v=%f\n", u, v);

    TouchScreenPoint out;

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

/**
 * @brief Rotates a point from the default GFX rotation (0) to the current GFX rotation
 * @param p Referenced point whose coordinates will rotate
 *
 * DISCUSSION:
 *  We assume the touchpad hardware coordinate system is always in the default GFX rotation (0).  We
 *  rotate the specified point's coordinates in-place to match the rotation actually in use by the
 *  GFX display driver.
 */
void TouchScreen::rotate(TouchScreenPoint& p) {
    float x = p.x;
    float y = p.y;

    // Query and implement the GFX rotation for the touchscreen
    int rotation = gfx.getRotation();
    switch (rotation) {
        // Default (portrait) --- TouchPad and screen are aligned
        case 0:
            DTRACE();
            break;

        // Landscape (90-degree clockwise)
        case 1:
            // p.x = gfx.height() - 1 - y;
            // p.y = x;
            p.x = y;
            p.y = gfx.height() - 1 - x;
            DTRACE();
            break;

        // Portrait (180-degree clockwise, upside-down)
        case 2:
            DTRACE();
            p.x = gfx.width() - 1 - x;
            p.y = gfx.height() - 1 - y;
            break;

        // Landscape (270-degree clockwise)
        case 3:
            p.x = gfx.width() - 1 - y;
            p.y = x;
            DTRACE();
            break;

        // Blunder
        default:
            DTRACE();
            break;
    }  // switch

    DPRINTF("Rot%d:  (%f,%f) ==> (%f,%f)\n", rotation, x, y, p.x, p.y);

}  // rotate()

/**
 * @brief Map a raw TouchPad coordinate to the screen coordinate system
 * @param raw TouchPad ADC X/Y coordinate
 * @param screen Resulting screen coordinate
 * @return true if successful
 *
 * DISCUSSION:
 *  This is where we invoke the bilinear interpolator to map raw ADC coordinates to
 *  screen pixel coordinates.  We perform the mapping without regard for the GFX
 *  rotation.  You can use rotate() to rotate the screen coordinate returned from
 *  mapRawToScreen().
 */
bool TouchScreen::mapRawToScreen(const TouchPadPoint& raw, TouchScreenPoint& screen) {
    TCZone cell;

    // The touchpad is divided into four zones known as "cells" having two rows
    // and two columns.  Interpolation will occur within one of these cells.
    if (!locateCell(raw, cell)) {
        DPRINTF("locateCell raw.x=%f raw.y=%f\n failed\n", raw.x, raw.y);
        return false;
    }

    // Use the calibration table to interpolate within the touched cell
    screen = bilinear(calibrationTable, cell);

    // Deal with out-of-bounds near edges
    if (screen.x < 0) screen.x = 0;
    if (screen.x > width - 1) screen.x = width - 1;
    if (screen.y < 0) screen.y = 0;
    if (screen.y >= height - 1) screen.y = height - 1;

    return true;
}  // mapRawToScreen()

/**
 * @brief Read touchscreen event
 * @param result Resulting screen coordinate of touch event if successful
 * @return true if successful
 *
 * DISCUSSION:
 *  Non-blocking read through TouchScreen, producing result in the screen
 *  coordinate system in the GFX current rotation.  Returns false if the TouchPad
 *  is not touched.  The result is filtered for noise, calibrated, and rotated into
 *  the screen coordinate system.
 */
bool TouchScreen::readTouchEvent(TouchScreenPoint& result) {
    TouchPadPoint raw;
    bool ok = touchPad.readFiltered(raw);  // Raw ADC coordinates
    if (ok) {
        ok = mapRawToScreen(raw, result);  // Calibrated map to screen coordinates
        if (ok) rotate(result);            // Rotate into current GFX rotation
    }
    return ok;
}
