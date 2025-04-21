#pragma once

#include <SPI.h>

#include "AColor.h"
#include "ACoord.h"
#include "ARect.h"
#include "Adafruit_GFX.h"
#include "HX8357_t3n.h"

class AGUI {
   public:
    // Constructors and initialization
    AGUI(HX8357_t3n *gfx, uint8_t rotation, const GFXfont *font);
    virtual ~AGUI() {}

    // AGUI really should have been a singleton
    AGUI(const AGUI &) = delete;
    AGUI &operator=(const AGUI &) = delete;

    // Graphics methods
    static void setClipRect(ACoord clipX, ACoord clipY, ACoord clipW, ACoord clipH);
    static void setClipRect(void);
    static void fillRect(ACoord x, ACoord y, ACoord w, ACoord h, AColor color);
    static void fillRoundRect(ACoord x, ACoord y, ACoord w, ACoord h, ACoord r, AColor color);
    static void drawRect(ACoord x, ACoord y, ACoord w, ACoord h, AColor color);
    static void drawRoundRect(ACoord x, ACoord y, ACoord w, ACoord h, ACoord r, AColor color);
    static void drawPixel(int16_t x, int16_t y, AColor color);

    // Text methods
    static void setCursor(ACoord x, ACoord y);
    static void setTextColor(AColor fg);
    static void setTextColor(AColor fg, AColor bg);
    static void setTextWrap(boolean w);
    static void setFont(const ILI9341_t3_font_t &f);
    static void setFont(const GFXfont *f);
    static void setFont(void);
    static ACoord getLeading(void);
    static size_t writeText(const uint8_t *buffer, size_t size);
    static size_t writeText(String str);
    static void getTextBounds(const uint8_t *buffer, uint16_t len, ACoord x, ACoord y,
                              ACoord *x1, ACoord *y1, ACoord *w, ACoord *h);
    static void getTextBounds(const char *string, ACoord x, ACoord y,
                              ACoord *x1, ACoord *y1, ACoord *w, ACoord *h);
    static void getTextBounds(const String &str, ACoord x, ACoord y,
                              ACoord *x1, ACoord *y1, ACoord *w, ACoord *h);
    static void setScrollTextArea(ACoord x, ACoord y, ALength w, ALength h);
    static void setScrollBackgroundColor(AColor color);
    static void enableScroll(void);
    static void disableScroll(void);
    static void scrollTextArea(uint8_t scrollSize);
    static void resetScrollBackgroundColor(AColor color);

    // Let's define some defaults for this application
    const static GFXfont *appFont;                      // Default font for this application
    const static AColor bgColor = AColor::A_BLACK;      // Application background color
    const static AColor fgColor = AColor::A_WHITE;      // Application foreground color
    const static AColor bdColor = AColor::A_DARK_GREY;  // Application border color
    const static AColor spColor = AColor::A_GREY;       // Application special color

    // Etc variables (all must be static or initialized in default constructor)
    static HX8357_t3n *gfx;  // Adafruit's GFX Library
    uint8_t screenRotation;  // See Adafruit HX8357 library doc for values
};
