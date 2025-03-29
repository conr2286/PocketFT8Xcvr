#pragma once

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"

class ADisplay {
   public:
    ADisplay();
    void begin(HX8357_t3n* gfx);
    static HX8357_t3n* tft;
};