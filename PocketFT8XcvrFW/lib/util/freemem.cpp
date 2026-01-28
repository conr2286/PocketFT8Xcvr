#include "freemem.h"
#include <malloc.h>

// Teensy 4.1 has 1MB of RAM (DTCM + OCRAM)
// DTCM: 512KB fast RAM at 0x20000000
// OCRAM: 512KB RAM at 0x20200000
// Additional PSRAM can be added externally

extern "C" char* sbrk(int i);
// Function to get free heap
uint32_t getFreeHeap() {
    struct mallinfo mi = mallinfo();
    return mi.fordblks;
}
