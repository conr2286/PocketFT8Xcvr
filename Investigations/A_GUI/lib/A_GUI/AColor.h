#pragma once

/**
 * AColor defines a small palette of colors for A_GUI widgets.  The
 * actual values of each color are coupled to the underlying display,
 * the HX8357 in the original implementation.  The design should be
 * able to support different displays so long as its colors can be
 * expressed as the integer values of an enumeration.
 */
#include <Adafruit_GFX.h>  //Must #include prior to HX8357
#include <SPI.h>

#include "HX8357_t3n.h"

// Bind AColors to a display.  The enumerated values must be correct for the
// display type (e.g. HX8357).  If you change display types, you must update
// these to config AGUI for the new display.
typedef enum {
    BLACK = HX8357_BLACK,
    RED = HX8357_RED,
    GREEN = HX8357_GREEN,
    BLUE = HX8357_BLUE,
    CYAN = HX8357_CYAN,
    MAGENTA = HX8357_MAGENTA,
    YELLOW = HX8357_YELLOW,
    ORANGE = HX8357_ORANGE,
    GREY = HX8357_DARKGREY,
    LIGHT_GREY = HX8357_LIGHTGREY,
    WHITE = HX8357_WHITE
} AColor;

// Define the default colors
#define DEFAULT_BACKGROUND_COLOR AColor::BLACK
#define DEFAULT_FOREGROUND_COLOR AColor::WHITE
#define DEFAULT_BORDER_COLOR AColor::RED
#define DEFAULT_SPECIAL_COLOR AColor::GREY
