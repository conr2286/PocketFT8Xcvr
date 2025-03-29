#include "ADisplay.h"

#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"

HX8357_t3n* ADisplay::tft=NULL;
ADisplay::ADisplay() { }
void ADisplay::begin(HX8357_t3n* gfx) { ADisplay::tft = gfx; }
