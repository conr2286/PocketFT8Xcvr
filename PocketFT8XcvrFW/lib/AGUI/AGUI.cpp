/**
 * AGUI isolates the widget implementations from the various Adafruit GFX library variations.
 * There's some minor added functionality but it's mostly an adapter to glue widgets to GFX.
 *  + Display types (e.g. TFT, etc)
 *  + Display controllers (e.g. HX8357, etc)
 *  + Display size (e.g. 480x320, etc)
 *  + Hardware interfaces (e.g. parallel, SPI)
 *  + MCU customization (e.g. Teensy)
 *  + Font support (GFX, ILI9341, Adafruit 5x7, etc)
 * While the isolation is not complete (fonts are a notable weakness), the task of switching
 * Adafruit libraries is largely confined to changes in just this class.
 *
 * IMPLEMENTATION
 * The original implementation of AGUI was for a 480x320 TFT display using the
 * HX8357D controller attached to a Teensy 4.1 through an SPI bus.  The AGUI code was
 * developed using Visual Studio Code, PlatformIO and many Arduino library dependencies.
 *
 * DESIGN NOTES
 *  + AGUI is heavily dependent upon the Arduino environment including the data types,
 *    classes (e.g. bool and String) and functions defined in Arduino.h
 *  + This is a minimalist approach with an Adafruit accent for a GUI library.  Consider
 *    LGVL for a more full-featured solution.
 *  + This adapter should probably become a singleton as multiple displays are forbidden
 *    due to the use of static member variables.
 *  + There is no dependency on nor interaction with the touchscreen system here.  See
 *    AWidget's static processTouch() method which is the bridge between the touch and
 *    display worlds.
 *  + Not every GFX method is adapted here.  You may need to implement something.
 *
 * DEPENDENCIES
 *  + https://github.com/adafruit/Adafruit_HX8357_Library
 *                          or
 *  + https://github.com/mjs513/HX8357_t3n
 *                          and
 *  + https://github.com/adafruit/Adafruit-GFX-Library
 *
 * REFERENCES
 * While on-line tutorials are available for the Adafruit GFX library, I've found minimal
 * complete documentation of the APIs and nothing but the source code for many versions on
 * github.  As of March, 2025, this is the best almost-complete API description I've found:
 * https://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___g_f_x.html
 * If the API you seek isn't in there, you'll have to resort to the your library's source.
 **/

#include "AGUI.h"

#include <SPI.h>

#include "Adafruit_GFX.h"  //HX8357_t3n requires you #include GFX before...
#include "HX8357_t3n.h"    //you #include the HX8357 variation.
#include "DEBUG.h"         //For printf-style debugging on a Teensy sans JTAG :(

//-----------------------------------------------------------------------------
HX8357_t3n* AGUI::gfx;
const GFXfont* AGUI::appFont;  // Default font for this application

//-----------------------------------------------------------------------------
//  Initialization
//-----------------------------------------------------------------------------

/**
 * @brief Construct the AGUI object
 * @param tft Pointer to an Adafruit display driver (e.g. HX8357_t3n) object
 * @param rotation GFX screen rotation parameter
 * @param font Pointer to a default GFX font structure
 *
 * @note The design imagined AGUI as a singleton (one display), however the
 * implementation didn't go out of its way to limit you to a single display.
 *
 */
AGUI::AGUI(HX8357_t3n* tft, uint8_t rotation, const GFXfont* font) {
    // if (!Serial) Serial.begin(9600);
    // Serial.print("AGUI()=");

    // Record configuration params
    gfx = tft;                  // Adafruit display object
    appFont = font;             // Application's default font
    screenRotation = rotation;  // See Adafruit HX8357 doc for values

    // Setup the Adafruit display and graphics library for use by our application
    gfx->begin(30000000UL, 2000000UL);  // Configure SPI clock speeds
    gfx->setRotation(rotation);         // Configure screen rotation
    gfx->setFont(appFont);              // Configure the font
    gfx->fillScreen(HX8357_BLACK);      // Erase the display

    // Serial.println("Exit AGUI\n");
}

//-----------------------------------------------------------------------------
//  Graphical methods: these *very* closely map into GFX.  Their implementation
//  here as a shim here attempts to provide some minimal independence of the
//  the higher-level code from the GFX display driver.
//
//  Coordinate and distance units are almost always given in pixels
//-----------------------------------------------------------------------------

/**
 * @brief Define the display's clip window rectangle
 * @param clipX x-Screen coordinate
 * @param clipY y-Screen coordinate
 * @param clipW width in pixels
 * @param clipH height in pixels
 *
 * @note GFX supports a single clip window
 */
void AGUI::setClipRect(ACoord clipX, ACoord clipY, ACoord clipW, ACoord clipH) {
    gfx->setClipRect(clipX, clipY, clipW, clipH);
}

/**
 * @brief Set the clip window to the full-screen
 */
void AGUI::setClipRect(void) {
    gfx->setClipRect();
}

/**
 * @brief Draw a single pixel of specified color
 * @param x Coordinate
 * @param y Coordinate
 * @param color Specified color
 */
void AGUI::drawPixel(int16_t x, int16_t y, AColor color) {
    gfx->drawPixel(x, y, color);
}

/**
 * @brief Fill a rectangle on the screen with the specified color
 * @param xCoord upper-left corner of rectangle
 * @param yCoord upper-left corner of rectangle
 * @param w width of rectangle
 * @param h height of rectangle
 * @param color Specified color
 */

void AGUI::fillRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, AColor color) {
    DPRINTF("xCoord=%d yCoord=%d, w=%d, h=%d, c=0x%04x\n", xCoord, yCoord, w, h, color);
    gfx->fillRect(xCoord, yCoord, w, h, color);
    // DTRACE();
}

/**
 * @brief Draws a filled, rounded rectangle on the screen with the specified color
 * @param xCoord Upper-left corner of rectangle
 * @param yCoord Upper-left corner of rectangle
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param r Radius of the rounded corners
 * @param color Fill color
 */
void AGUI::fillRoundRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, ACoord r, AColor color) {
    gfx->fillRoundRect(xCoord, yCoord, w, h, r, color);
}

/**
 * @brief Draw outline of a rectangle on the screen with the specified color
 * @param xCoord upper-left corner of rectangle
 * @param yCoord upper-left corner of rectangle
 * @param w width of rectangle
 * @param h height of rectangle
 * @param color Specified color
 */
void AGUI::drawRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, AColor color) {
    gfx->drawRect(xCoord, yCoord, w, h, color);
}

/**
 * @brief Draw border of a rounded rectangle on the screen with the specified color
 * @param xCoord Upper-left corner of rectangle
 * @param yCoord Upper-left corner of rectangle
 * @param w Width
 * @param h Height
 * @param r Radius of the rounded corners
 * @param color Border color
 */
void AGUI::drawRoundRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, ACoord r,
                         AColor color) {
    gfx->drawRoundRect(xCoord, yCoord, w, h, r, color);
}

//-----------------------------------------------------------------------------
//  Text methods.  The underlying GFX library supports multiple font systems,
//  GFX and ILI9341, which are not abstracted by AGUI.
//-----------------------------------------------------------------------------

/**
 * @brief Select the specified GFX font
 * @param f Pointer to the GFXfont struct
 */
void AGUI::setFont(const GFXfont* f) {
    gfx->setFont(f);
}

/**
 * @brief Select the specified GFX font
 */
void AGUI::setFont(void) {
    gfx->setFont();
}

/**
 * @brief Select the specified ILI9341 font
 * @param f Pointer to an ILI9341_t3_font struct
 */
void AGUI::setFont(const ILI9341_t3_font_t& f) {
    gfx->setFont(f);
}

/**
 * @brief Get the leading for the selected font
 * @return Number of pixels between lines
 *
 * @note "Leading" refers to the number of pixels between lines including the height
 * of the text and the white space between lines
 *
 * @note You may have to modify or subclass the GFX library to implement this because
 * the required font metrics for the calculation are protected in Adafruit_GFX.   If
 * you *really* don't want to do this, you could just cast-it-in-brass here if you're
 * only using a single font.
 */
ACoord AGUI::getLeading() {
    DTRACE();
    return gfx->getLeading();
}

/**
 * @brief Place the text cursor at the specified coordinate
 * @param xCoord coordinate
 * @param yCoord coordinate
 */
void AGUI::setCursor(ACoord xCoord, ACoord yCoord) {
    gfx->setCursor(xCoord, yCoord);
}

/**
 * @brief Set the text foreground color
 * @param fg Specified color
 *
 * @note Subsequent text will be displayed in the specified color
 */
void AGUI::setTextColor(AColor fg) {
    gfx->setTextColor(fg);
}

/**
 * @brief Set the text foreground and background colors
 * @param fg Foreground color
 * @param bg Background color
 *
 * @note Subsequent text will be displayed in the specified colors
 */
void AGUI::setTextColor(AColor fg, AColor bg) {
    gfx->setTextColor(fg, bg);
}

/**
 * @brief Specify whether text should wrap
 * @param w true==>wrap, false if not
 */
void AGUI::setTextWrap(bool w) {
    gfx->setTextWrap(w);
}

/**
 * @brief Writes size chars from buffer to the display
 * @param buffer Characters to write
 * @param size #chars to write
 * @return #chars actually written
 *
 * We seem to be writing 7-bit ASCII chars, written left-to-write, starting
 * at the current cursor position.
 */
size_t AGUI::writeText(const uint8_t* buffer, size_t size) {
    return gfx->write(buffer, size);
}

/**
 * @brief Write Arduino String to the display
 * @param str The String to write
 * @return #chars actually written
 */
size_t AGUI::writeText(String str) {
    return gfx->write((const uint8_t*)str.c_str(), str.length());
}

/**
 * @brief Get the boundary rectangle for the specified character string buffer
 * @param buffer The characters
 * @param len #chars in buffer
 * @param x Specify the upper-left coordinate of the boundary rectangle
 * @param y Specify the upper-left coordinate of the boundary rectangle
 * @param x1 Calculated lower-right coordinate of the boundary rectangle
 * @param y1 Calculated lower-right coordinate of the boundary rectangle
 * @param w Calculated width of the boundary rectangle
 * @param h Calculated height of the boundary rectangle
 */
void AGUI::getTextBounds(const uint8_t* buffer, uint16_t len, ACoord x, ACoord y,
                         ACoord* x1, ACoord* y1, ACoord* w, ACoord* h) {
    gfx->getTextBounds(buffer, len, (int16_t)x, (int16_t)y, (int16_t*)x1, (int16_t*)y1, (uint16_t*)w, (uint16_t*)h);
}

/**
 * @brief Get the boundary rectangle for the specified character string
 * @param string NUL-terminated char[] string
 * @param x Specify the upper-left coordinate of the boundary rectangle
 * @param y Specify the upper-left coordinate of the boundary rectangle
 * @param x1 Calculated lower-right coordinate of the boundary rectangle
 * @param y1 Calculated lower-right coordinate of the boundary rectangle
 * @param w Calculated width of the boundary rectangle
 * @param h Calculated height of the boundary rectangle
 */
void AGUI::getTextBounds(const char* string, ACoord x, ACoord y,
                         ACoord* x1, ACoord* y1, ACoord* w, ACoord* h) {
    gfx->getTextBounds(string, (int16_t)x, (int16_t)y, (int16_t*)x1, (int16_t*)y1, (uint16_t*)w, (uint16_t*)h);
}

/**
 * @brief Get the boundary rectangle for the specified String
 * @param str String object
 * @param x Specify the upper-left coordinate of the boundary rectangle
 * @param y Specify the upper-left coordinate of the boundary rectangle
 * @param x1 Calculated lower-right coordinate of the boundary rectangle
 * @param y1 Calculated lower-right coordinate of the boundary rectangle
 * @param w Calculated width of the boundary rectangle
 * @param h Calculated height of the boundary rectangle
 */
void AGUI::getTextBounds(const String& str, ACoord x, ACoord y,
                         ACoord* x1, ACoord* y1, ACoord* w, ACoord* h) {
    gfx->getTextBounds(str, (int16_t)x, (int16_t)y, (int16_t*)x1, (int16_t*)y1, (uint16_t*)w, (uint16_t*)h);
}  // getTextBounds()

/**
 * @brief Set the scrollable text area rectangle
 */
void AGUI::setScrollTextArea(ACoord x, ACoord y, ALength w, ALength h) {
    gfx->setScrollTextArea(x, y, w, h);
}

/**
 * @brief Fills the scroll text area with the specified color
 * @param color Background
 */
void AGUI::setScrollBackgroundColor(AColor color) {
    gfx->setScrollBackgroundColor(color);
}

/**
 * @brief Enable text scrolling
 */
void AGUI::enableScroll() {
    gfx->enableScroll();
}

/**
 * @brief Disable text scrolling
 */
void AGUI::disableScroll() {
    gfx->disableScroll();
}

/**
 * @brief Scroll the previously defined text area rectangle
 * @param scrollSize Number of pixel rows to scroll
 *
 * @note HX8357_t3n implements scrolling by reading and re-writing rows of pixels
 * and filling in rows of scroll background color below.
 */
void AGUI::scrollTextArea(uint8_t scrollSize) {
    gfx->scrollTextArea(scrollSize);
}

/**
 * @brief Reconfigures the scroll area's background color but doesn't draw anything
 * @param color The new color configuration
 */
void AGUI::resetScrollBackgroundColor(AColor color) {
    gfx->resetScrollBackgroundColor(color);
}
