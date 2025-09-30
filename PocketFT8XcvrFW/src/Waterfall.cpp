#include "Waterfall.h"

#include <Arduino.h>

#include "Process_DSP.h"
#include "Station.h"


// Define the three frequencies associated with the Waterfall
//uint16_t cursor_freq;      // Frequency of cursor line
uint16_t cursor_line;      // Pixel location of cursor line
//int offset_freq;           // TOOD:  Deprecated???
int start_up_offset_freq;  // TODO:  Deprecate???
extern Station thisStation; // Station params

/**
 * @brief Initialize cursor frequencies
 */
void set_startup_freq(void) {
    cursor_line = 112;
    // start_up_offset_freq = EEPROMReadInt(10);     //Charlie
    start_up_offset_freq = 112;  // KQ7B
    thisStation.setCursorFreq((uint16_t)((float)(cursor_line + ft8_min_bin) * FFT_Resolution));
    //offset_freq = start_up_offset_freq;
    // DPRINTF("set_startup_freq:  start_up_offset_freq=%d, cursor_freq=%d, offset_freq=%d\n", start_up_offset_freq, cursor_freq, offset_freq);
}  // set_startup_freq()