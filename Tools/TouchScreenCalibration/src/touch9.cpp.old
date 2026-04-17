#include "touch9.h"
#include <EEPROM.h>
#include <DEBUG.h>
#include "touchpad.h"

// ---------- Internal globals ----------

static T9Calib9 g_calib;
static bool g_calib_valid = false;

// 9 target positions for 320x480
static const T9Point T9_TARGETS[9] = {
    {0, 0}, {160, 0}, {319, 0}, {0, 240}, {160, 240}, {319, 240}, {0, 479}, {160, 479}, {319, 479}};

// EEPROM layout
static constexpr int EEPROM_MAGIC_ADDR = 0;
static constexpr int EEPROM_DATA_ADDR = 4;
static constexpr uint32_t EEPROM_MAGIC = 0x39544339;  // "9TC9"

// ---------- Low-level touch reading (Teensy 4.1) ----------
// 4-wire resistive panel: XP, XM, YP, YM

static void t9_pins_init() {
    pinMode(T9Pins::XP, INPUT);
    pinMode(T9Pins::XM, INPUT);
    pinMode(T9Pins::YP, INPUT);
    pinMode(T9Pins::YM, INPUT);
}

// // Read X coordinate (0..1023)
// static uint16_t t9_read_x() {
//     // Drive Y+ high, Y- low, read X+
//     pinMode(T9Pins::YP, OUTPUT);
//     pinMode(T9Pins::YM, OUTPUT);
//     digitalWrite(T9Pins::YP, HIGH);
//     digitalWrite(T9Pins::YM, LOW);

//     pinMode(T9Pins::XP, INPUT);
//     pinMode(T9Pins::XM, INPUT);

//     delayMicroseconds(20);
//     return analogRead(T9Pins::XP);
// }

// // Read Y coordinate (0..1023)
// static uint16_t t9_read_y() {
//     // Drive X+ high, X- low, read Y+
//     pinMode(T9Pins::XP, OUTPUT);
//     pinMode(T9Pins::XM, OUTPUT);
//     digitalWrite(T9Pins::XP, HIGH);
//     digitalWrite(T9Pins::XM, LOW);

//     pinMode(T9Pins::YP, INPUT);
//     pinMode(T9Pins::YM, INPUT);

//     delayMicroseconds(20);
//     return analogRead(T9Pins::YP);
// }

// // Crude pressure estimate (Z)
// static uint16_t t9_read_z() {
//     // Simple proxy; you can refine with proper resistance measurement
//     return t9_read_x();
// }

// ---------- Public driver API ----------

void t9_init() {
    t9_pins_init();
    analogReadResolution(10);  // 10-bit ADC
}

bool t9_read_raw(T9Point& raw, uint16_t& z) {
    // uint16_t x = t9_read_x();
    // uint16_t y = t9_read_y();
    // uint16_t zz = t9_read_z();

    TouchPoint p = getTouchPoint();
    uint16_t x = p.x;
    uint16_t y = p.y;
    uint16_t zz = p.z;

    // Simple threshold for "touch present"
    if (zz < 50) {
        return false;
    }

    raw.x = x;
    raw.y = y;
    z = zz;
    DPRINTF("t9_read_raw() returns raw.x=%f raw.y=%f z=%d\n", raw.x, raw.y, z);

    return true;
}

// ---------- Small median helper ----------

template <int N>
static void sort_small(uint16_t (&a)[N]) {
    for (int i = 1; i < N; i++) {
        uint16_t v = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > v) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = v;
    }
}

// ---------- Filtered read ----------

bool t9_read_filtered(T9Point& raw, uint16_t& z) {
    // const int N = 7;
    static const int N = 3;
    uint16_t xs[N], ys[N], zs[N];

    for (int i = 0; i < N; i++) {
        T9Point r;
        uint16_t zz;
        if (!t9_read_raw(r, zz)) {
            return false;  // touch lifted
        }
        xs[i] = (uint16_t)r.x;
        ys[i] = (uint16_t)r.y;
        zs[i] = zz;
        delayMicroseconds(300);
    }

    sort_small(xs);
    sort_small(ys);
    sort_small(zs);

    raw.x = xs[N / 2];
    raw.y = ys[N / 2];
    z = zs[N / 2];
    DPRINTF("t9_read_filtered returns raw.x=%f raw.y=%f z=%d\n", raw.x, raw.y, z);
    return true;
}

// ---------- Calibration state machine ----------

static T9CalState g_cal_state = T9CalState::Idle;
static int g_cal_index = 0;
static int g_stable_count = 0;
static const int g_required_stable = 6;
static T9DrawTargetFn g_draw_target = nullptr;

static bool t9_is_touching() {
    // uint16_t z = t9_read_z();
    TouchPoint p = getTouchPoint();
    return p.z;
    // return z > 50;
}

void t9_calib_start(T9DrawTargetFn drawFn) {
    g_cal_state = T9CalState::Running;
    g_cal_index = 0;
    g_stable_count = 0;
    g_draw_target = drawFn;
    g_calib_valid = false;

    if (g_draw_target) {
        g_draw_target(g_cal_index, T9_TARGETS[g_cal_index]);
    }
}

void t9_calib_update() {
    if (g_cal_state != T9CalState::Running)
        return;

    // If no touch, reset stability counter and wait
    if (!t9_is_touching()) {
        g_stable_count = 0;
        return;
    }

    T9Point raw;
    uint16_t z;
    if (!t9_read_filtered(raw, z)) {
        g_stable_count = 0;
        return;
    }

    g_stable_count++;
    if (g_stable_count < g_required_stable)
        return;

    // Accept this point
    g_calib.nodes[g_cal_index].raw = raw;
    g_calib.nodes[g_cal_index].screen = T9_TARGETS[g_cal_index];

    g_cal_index++;
    g_stable_count = 0;

    if (g_cal_index >= 9) {
        g_cal_state = T9CalState::Done;
        g_calib_valid = true;
    } else {
        if (g_draw_target) {
            g_draw_target(g_cal_index, T9_TARGETS[g_cal_index]);
        }
    }

    delay(50);
}

T9CalState t9_calib_state() {
    return g_cal_state;
}

const T9Calib9& t9_calib_get() {
    return g_calib;
}

// ---------- EEPROM save/load ----------

bool t9_calib_save() {
    EEPROM.put(EEPROM_DATA_ADDR, g_calib);
    EEPROM.put(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
    // EEPROM.commit();
    return true;
}

bool t9_calib_load(T9Calib9& out) {
    uint32_t magic = 0;
    EEPROM.get(EEPROM_MAGIC_ADDR, magic);
    if (magic != EEPROM_MAGIC)
        return false;

    EEPROM.get(EEPROM_DATA_ADDR, out);
    g_calib = out;
    g_calib_valid = true;
    return true;
}

// ---------- Mapping: raw -> screen (bilinear) ----------

struct T9Cell {
    int row;
    int col;
    float u;
    float v;
};

static const T9CalibNode& nodeAt(const T9Calib9& c, int r, int cidx) {
    return c.nodes[r * 3 + cidx];
}

static bool locateCell(const T9Calib9& cal, const T9Point& raw, T9Cell& cell) {
    int row = -1;
    int col = -1;

    DPRINTF("raw.x=%f raw.y=%f\n", raw.x, raw.y);

    // Find row band
    for (int r = 0; r < 2; r++) {
        float y0 = nodeAt(cal, r, 0).raw.y;
        float y1 = nodeAt(cal, r + 1, 0).raw.y;
        if ((raw.y >= y0 && raw.y <= y1) || (raw.y <= y0 && raw.y >= y1)) {
            row = r;
            break;
        }
    }

    // Find column band
    for (int c = 0; c < 2; c++) {
        float x0 = nodeAt(cal, 0, c).raw.x;
        float x1 = nodeAt(cal, 0, c + 1).raw.x;
        if ((raw.x >= x0 && raw.x <= x1) || (raw.x <= x0 && raw.x >= x1)) {
            col = c;
            break;
        }
    }

    // Fudge row/col to accomodate less than perfect calibration result
    DPRINTF("found row=%d col=%d\n", row, col);
    if (row < 0) row = 0;
    if (col < 0) col = 0;
    DPRINTF("adjusted row=%d col=%d\n", row, col);

    // Original Copilot code rejected bad result
    if (row < 0 || col < 0) {
        DTRACE();
        return false;
    }

    const T9CalibNode& n00 = nodeAt(cal, row, col);
    const T9CalibNode& n10 = nodeAt(cal, row, col + 1);
    const T9CalibNode& n01 = nodeAt(cal, row + 1, col);

    float dx = n10.raw.x - n00.raw.x;
    float dy = n01.raw.y - n00.raw.y;
    if (dx == 0.0f || dy == 0.0f) {
        DTRACE();
        return false;
    }

    float u = (raw.x - n00.raw.x) / dx;
    float v = (raw.y - n00.raw.y) / dy;

    cell.row = row;
    cell.col = col;
    cell.u = u;
    cell.v = v;
    DPRINTF("locateCell() returns cell.row=%d cell.col=%d cell.u=%f cell.f=%f\n", cell.row, cell.col, cell.u, cell.v);
    return true;
}

static T9Point bilinear(const T9Calib9& cal, const T9Cell& cell) {
    const T9CalibNode& n00 = nodeAt(cal, cell.row, cell.col);
    const T9CalibNode& n10 = nodeAt(cal, cell.row, cell.col + 1);
    const T9CalibNode& n01 = nodeAt(cal, cell.row + 1, cell.col);
    const T9CalibNode& n11 = nodeAt(cal, cell.row + 1, cell.col + 1);

    float u = cell.u;
    float v = cell.v;

    T9Point out;
    out.x =
        (1.0f - u) * (1.0f - v) * n00.screen.x +
        (u) * (1.0f - v) * n10.screen.x +
        (1.0f - u) * (v)*n01.screen.x +
        (u) * (v)*n11.screen.x;

    out.y =
        (1.0f - u) * (1.0f - v) * n00.screen.y +
        (u) * (1.0f - v) * n10.screen.y +
        (1.0f - u) * (v)*n01.screen.y +
        (u) * (v)*n11.screen.y;

    return out;
}

bool t9_map_raw_to_screen(const T9Calib9& cal, const T9Point& raw, T9Point& screen) {
    T9Cell cell;
    if (!locateCell(cal, raw, cell)) {
        DTRACE();
        return false;
    }
    screen = bilinear(cal, cell);
    DPRINTF("t9_map_raw_to_screen() calc'd screen.x=%f screen.y=%f\n", screen.x, screen.y);

    // Workaround imperfect calibration near edges
    if (screen.x < 0) screen.x = 0;
    if (screen.x >= T9_SCREEN_WIDTH) screen.x = T9_SCREEN_WIDTH - 1;
    if (screen.y < 0) screen.y = 0;
    if (screen.y >= T9_SCREEN_HEIGHT) screen.y = T9_SCREEN_HEIGHT - 1;
    DPRINTF("t9_map_raw_to_screen() adjusted screen.x=%f screen.y=%f\n", screen.x, screen.y);
    return true;
}
