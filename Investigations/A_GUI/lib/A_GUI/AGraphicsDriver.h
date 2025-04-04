#pragma once

#include <SPI.h>

#include "DEBUG.h"

#include "AColor.h"
#include "ACoord.h"
#include "ARect.h"
#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"

class AGraphicsDriver {
   public:
    // Constructors and initialization
    AGraphicsDriver(void);
    void begin(HX8357_t3n *gfx);

    // Graphics methods
    void setClipRect(ACoord clipX, ACoord clipY, ACoord clipW, ACoord clipH);
    void setClipRect(void);
    void fillRect(ACoord x, ACoord y, ACoord w, ACoord h, AColor color);
    void drawRect(ACoord x, ACoord y, ACoord w, ACoord h, AColor color);
    void drawPixel(int16_t x, int16_t y, AColor color);


    // Text methods
    void setCursor(ACoord x, ACoord y);
    void setTextColor(AColor fg);
    void setTextColor(AColor fg, AColor bg);
    void setTextWrap(boolean w);
    void setFont(const ILI9341_t3_font_t &f);
    void setFont(const GFXfont *f);
    void setFont(void);
    ACoord getLeading(void);
    size_t writeText(const uint8_t *buffer, size_t size);
    void getTextBounds(const uint8_t *buffer, uint16_t len, int16_t x, int16_t y,
                       int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const char *string, int16_t x, int16_t y,
                       int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const String &str, int16_t x, int16_t y,
                       int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

    // Variables (all must be static or initialized in default constructor)
     static HX8357_t3n *gfx;  // Adafruit's GFX Library
     const static GFXfont *txtFont;  // Default font for this application
};