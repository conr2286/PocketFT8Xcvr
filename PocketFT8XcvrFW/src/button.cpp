
#include "button.h"

#include <Arduino.h>

#include "HX8357_t3n.h"
#include "NODEBUG.h"
#include "decode_ft8.h"
// #include "display.h"
#include "gen_ft8.h"
#include "traffic_manager.h"
// #include <Encoder.h>
#include <EEPROM.h>
#include <SI4735.h>
#include <Wire.h>

#include "Process_DSP.h"
#include "Sequencer.h"
#include "TouchScreen_I2C.h"
#include "msgTypes.h"
#include "pins.h"

// Define which I2C bus we are using
#define WIRE WIRE_ETC

// This is calibration data for the raw touch data to the screen coordinates
// using 510 Ohm resistors to reduce the driven voltage to Y+ and X-
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130

#define MINPRESSURE 120
#define PENRADIUS 3

extern HX8357_t3n tft;
extern TouchScreen ts;

extern int Transmit_Armned;

extern SI4735 si4735;
#define USB 2

uint16_t draw_x, draw_y, touch_x, touch_y;
int test;

extern int master_decoded;
extern void sync_FT8(void);
// extern uint16_t cursor_freq;
extern int tune_flag;
// extern uint16_t cursor_line;
// extern int offset_freq;

const float ft8_shift = 6.25;  // FT8 Hz/bin???
// int start_up_offset_freq;
extern int log_flag, logging_on;

// Get a reference to the Sequencer singleton
static Sequencer& seq = Sequencer::getSequencer();

// Reference the GPS wait-for-it synchronizer
// void waitForFT8timeslot(void);  chh thinks this is unnesscary

// Setter for Sequencer's RoboOp response to a received CQ message
void setAutoReplyToCQ(bool);

#define numButtons 9
#define button_height BUTTON_BAR_H  // TODO:  Clean-up old definitions for height and top line location
#define button_line BUTTON_BAR_Y
#define button_width 42

/**
 * @brief Dis-arms the transmitter, switches from xmit t recv, and clears the outbound FT8 message
 */
void terminate_transmit_armed(void) {
    Transmit_Armned = 0;
    receive_sequence();
    // sButtonData[6].state = false;
    // drawButton(6);
}

#define IOEXP16_ADDR 0x24
