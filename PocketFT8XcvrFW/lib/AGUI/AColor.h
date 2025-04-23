#pragma once

/**
 * AColor defines a small palette of colors for AGUI widgets.  The
 * actual values of each color are coupled to the underlying display,
 * the HX8357 in the original implementation.  This design should be
 * able to support different displays so long as its colors can be
 * expressed as the integer (int16, int32...) values of an enumeration.
 */
#include <Adafruit_GFX.h>  //Must #include prior to HX8357
#include <SPI.h>

#include "HX8357_t3n.h"

// Bind AColors to a display.  The enumerated values must be correct for the
// display type (e.g. HX8357).  If you change display types, you must update
// these to config AGUI for the new display.
typedef enum {
    A_BLACK = HX8357_BLACK,
    A_RED = HX8357_RED,
    A_GREEN = HX8357_GREEN,
    A_DARK_GREEN= HX8357_DARKGREEN,
    A_BLUE = HX8357_BLUE,
    A_CYAN = HX8357_CYAN,
    A_MAGENTA = HX8357_MAGENTA,
    A_YELLOW = HX8357_YELLOW,
    A_ORANGE = HX8357_ORANGE,
    A_GREY = HX8357_DARKGREY,
    A_LIGHT_GREY = HX8357_LIGHTGREY,
    A_DARK_GREY = A_GREY,
    A_WHITE = HX8357_WHITE
} AColor;

// Define the default colors
#define DEFAULT_BACKGROUND_COLOR AColor::A_BLACK
#define DEFAULT_FOREGROUND_COLOR AColor::A_WHITE
#define DEFAULT_BORDER_COLOR AColor::A_RED
#define DEFAULT_SPECIAL_COLOR AColor::A_YELLOW
#define ALT_BACKGROUND_COLOR AColor::A_GREY
