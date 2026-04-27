#include "TouchCalibrator.h"
// #include <EEPROM.h>
#include "hwdefs.h"
#include "TouchPad.h"
#include "DEBUG.h"

// ---------- Internal helpers ----------

// 9 target positions for 320x480
static const int N_TARGETS = 9;
static const int TARGET_OFFSET = 32;  // Target offset in pixels from display edges
static const TCPoint theTargetCoordinates[N_TARGETS] = {
    {0, 0}, {160, 0}, {319, 0}, {0, 240}, {160, 240}, {319, 240}, {0, 479}, {160, 479}, {319, 479}};
// static const TCPoint theTargetCoordinates[N_TARGETS] = {
//     {TARGET_OFFSET, TARGET_OFFSET}, {160, TARGET_OFFSET}, {319 - TARGET_OFFSET, TARGET_OFFSET}, {TARGET_OFFSET, 240}, {160, 240}, {319 - TARGET_OFFSET, 240}, {TARGET_OFFSET, 479 - TARGET_OFFSET}, {160, 479 - TARGET_OFFSET}, {319 - TARGET_OFFSET, 479 - TARGET_OFFSET}};

// Build the calibration table
static TouchCalibrationTable theCalibrationTable;
static bool validCalibration = false;

// EEPROM layout
static constexpr int EEPROM_MAGIC_ADDR = 0;
static constexpr int EEPROM_DATA_ADDR = 4;
static constexpr uint32_t EEPROM_MAGIC = 0x39544339;  // "9TC9"

// Getters
unsigned getNTargets(void) { return N_TARGETS; }
TCPoint getTargetCoordinate(unsigned idx) { return theTargetCoordinates[idx]; }

// Setter binds a target node's ADC readings to its screen coordinates in the calibration table
void recordCalibrationNode(unsigned idx, TCPoint adc) {
    if (idx >= getNTargets()) return;                                  // Sanity check
    theCalibrationTable.nodes[idx].screen = getTargetCoordinate(idx);  // The target's screen coordinates
    theCalibrationTable.nodes[idx].raw = adc;                          // The touchpad's ADC coordinate values
    DPRINTF("Node[%d] maps touchpoint (%f,%f) to screen (%f,%f)\n", idx,
            theCalibrationTable.nodes[idx].raw.x, theCalibrationTable.nodes[idx].raw.y,
            theCalibrationTable.nodes[idx].screen.x, theCalibrationTable.nodes[idx].screen.y);
}

// ---------- Low-level touch reading (Teensy 4.1) ----------
// Simple 4-wire resistive panel: PIN_XP, XM, PIN_YP, PIN_YM

// static void t9_pins_init() {
//     pinMode(PIN_XP, INPUT);
//     pinMode(PIN_XM, INPUT);
//     pinMode(PIN_YP, INPUT);
//     pinMode(PIN_YM, INPUT);
// }

// // Read X coordinate (0..1023)
// static uint16_t t9_read_x() {
//     // Drive Y+ high, Y- low, read X+
//     pinMode(PIN_YP, OUTPUT);
//     pinMode(PIN_YM, OUTPUT);
//     digitalWrite(PIN_YP, HIGH);
//     digitalWrite(PIN_YM, LOW);

//     pinMode(PIN_XP, INPUT);
//     pinMode(PIN_XM, INPUT);

//     delayMicroseconds(20);
//     return analogRead(PIN_XP);  // 10-bit default
// }

// // Read Y coordinate (0..1023)
// static uint16_t t9_read_y() {
//     // Drive X+ high, X- low, read Y+
//     pinMode(PIN_XP, OUTPUT);
//     pinMode(PIN_XM, OUTPUT);
//     digitalWrite(PIN_XP, HIGH);
//     digitalWrite(PIN_XM, LOW);

//     pinMode(PIN_YP, INPUT);
//     pinMode(PIN_YM, INPUT);

//     delayMicroseconds(20);
//     return analogRead(PIN_YP);
// }

// // Crude pressure estimate (Z)
// static uint16_t t9_read_z() {
//     // One simple approach: measure resistance between X and Y
//     // Here we just reuse X reading as a proxy; you can refine this.
//     return t9_read_x();
// }

// // ---------- Public driver API ----------

// void t9_init() {
//     t9_pins_init();
//     analogReadResolution(10);  // 10-bit
// }
//
// bool t9_read_raw(TCPoint& raw, uint16_t& z) {
//     uint16_t x = t9_read_x();
//     uint16_t y = t9_read_y();
//     uint16_t zz = t9_read_z();

//     // Simple threshold for "touch present"
//     if (zz < 50) {
//         return false;
//     }

//     raw.x = x;
//     raw.y = y;
//     z = zz;
//     return true;
// }

// // Small median helper
// template <int N>
// static void sort_small(uint16_t (&a)[N]) {
//     for (int i = 1; i < N; i++) {
//         uint16_t v = a[i];
//         int j = i - 1;
//         while (j >= 0 && a[j] > v) {
//             a[j + 1] = a[j];
//             j--;
//         }
//         a[j + 1] = v;
//     }
// }
static TouchPad touchPad = TouchPad(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_XR, PIN_YR);
bool t9_read_filtered(TCPoint& result, uint16_t& z) {
    TouchPoint raw = touchPad.getTouchEvent();
    if (raw.z == TS_NO_TOUCH) return false;
    result.x = raw.x;
    result.y = raw.y;
    z = (raw.x + raw.y) / 2;
    return true;
}

// bool t9_read_filtered(TCPoint& raw, uint16_t& z) {
//     const int N = 7;
//     uint16_t xs[N], ys[N], zs[N];

//     int count = 0;
//     for (int i = 0; i < N; i++) {
//         TCPoint r;
//         uint16_t zz;
//         if (!t9_read_raw(r, zz)) {
//             // no touch
//             return false;
//         }
//         xs[count] = (uint16_t)r.x;
//         ys[count] = (uint16_t)r.y;
//         zs[count] = zz;
//         count++;
//         delayMicroseconds(300);
//     }

//     sort_small(xs);
//     sort_small(ys);
//     sort_small(zs);

//     raw.x = xs[N / 2];
//     raw.y = ys[N / 2];
//     z = zs[N / 2];
//     return true;
// }

// ---------- Calibration state machine ----------

static T9CalState g_cal_state = T9CalState::Idle;
static int g_cal_index = 0;
static int g_stable_count = 0;
static const int g_required_stable = 6;
static T9DrawTargetFn g_draw_target = nullptr;

static bool t9_is_touching() {
    // uint16_t z = t9_read_z();
    // return z > 50;
    TouchPoint p = touchPad.getTouchPoint();
    return p.z == TS_TOUCH;
}

void t9_calib_start(T9DrawTargetFn drawFn) {
    g_cal_state = T9CalState::Running;
    g_cal_index = 0;
    g_stable_count = 0;
    g_draw_target = drawFn;
    validCalibration = false;

    if (g_draw_target) {
        g_draw_target(g_cal_index, theTargetCoordinates[g_cal_index]);
    }
}

void t9_calib_update() {
    if (g_cal_state != T9CalState::Running)
        return;

    if (!t9_is_touching()) {
        g_stable_count = 0;
        return;
    }

    TCPoint raw;
    uint16_t z;
    if (!t9_read_filtered(raw, z)) {
        g_stable_count = 0;
        return;
    }

    g_stable_count++;
    if (g_stable_count < g_required_stable)
        return;

    // Accept this point
    theCalibrationTable.nodes[g_cal_index].raw = raw;
    theCalibrationTable.nodes[g_cal_index].screen = theTargetCoordinates[g_cal_index];

    g_cal_index++;
    g_stable_count = 0;

    if (g_cal_index >= 9) {
        g_cal_state = T9CalState::Done;
        validCalibration = true;
    } else {
        if (g_draw_target) {
            g_draw_target(g_cal_index, theTargetCoordinates[g_cal_index]);
        }
    }
}

T9CalState t9_calib_state() {
    return g_cal_state;
}

const TouchCalibrationTable& t9_calib_get() {
    return theCalibrationTable;
}

// ---------- EEPROM save/load ----------

bool t9_calib_save() {
    // EEPROM.put(EEPROM_DATA_ADDR, theCalibrationTable);
    // EEPROM.put(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
    // EEPROM.commit();
    return true;
}

bool t9_calib_load(TouchCalibrationTable& out) {
    // uint32_t magic = 0;
    // EEPROM.get(EEPROM_MAGIC_ADDR, magic);
    // if (magic != EEPROM_MAGIC)
    //     return false;

    // EEPROM.get(EEPROM_DATA_ADDR, out);
    // theCalibrationTable = out;
    // validCalibration = true;
    return true;
}

// ---------- Mapping: raw -> screen (bilinear) ----------
// We divide the touchpad into four TouchCalibrator Zones and bilinear interpolate within each
struct TCZone {
    int row;  // This zone's row 0..1
    int col;  // This zone's col 0..1
    float u;
    float v;
};

/**
 * @brief Find the calibration table node for row/col
 * @param c Calibration table
 * @param r Row index 0..2
 * @param cidx Column index 0..2
 * @return Calibration node for row/col
 */
static const TouchCalibrationNode& nodeAt(const TouchCalibrationTable& c, int r, int cidx) {
    // Sanity checks
    if ((r < 0) || (r >= 3) || (cidx < 0) || (cidx >= 3)) {
        DTRACE();
    }
    return c.nodes[r * 3 + cidx];
}

/**
 * @brief Return the maximum of three values
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
static bool locateCell(const TouchCalibrationTable& cal, const TCPoint& raw, TCZone& cell) {
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
static TCPoint bilinear(const TouchCalibrationTable& cal, const TCZone& cell) {
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

bool mapRawToScreen(const TCPoint& raw, TCPoint& screen) {
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
