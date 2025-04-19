#include "Waterfall.h"

#include <Arduino.h>

#include "Process_DSP.h"


// Define the three frequencies associated with the Waterfall
uint16_t cursor_freq;      // Frequency of cursor line
uint16_t cursor_line;      // Pixel location of cursor line
int offset_freq;           // TOOD:  Deprecated???
int start_up_offset_freq;  // TODO:  Deprecate???

/**
 * @brief Initialize cursor frequencies
 */
void set_startup_freq(void) {
    cursor_line = 100;
    // start_up_offset_freq = EEPROMReadInt(10);     //Charlie
    start_up_offset_freq = 0;  // KQ7B
    cursor_freq = (uint16_t)((float)(cursor_line + ft8_min_bin) * FFT_Resolution);
    offset_freq = start_up_offset_freq;
    // DPRINTF("set_startup_freq:  start_up_offset_freq=%d, cursor_freq=%d, offset_freq=%d\n", start_up_offset_freq, cursor_freq, offset_freq);
}  // set_startup_freq()