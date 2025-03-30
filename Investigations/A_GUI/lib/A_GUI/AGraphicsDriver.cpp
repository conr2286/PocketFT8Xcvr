/**
 * AGraphicsDriver isolates the AGUI widgets from the various Adafruit GFX library variations:
 *  + Display types (e.g. TFT, etc)
 *  + Display controllers (e.g. HX8357, etc)
 *  + Display size (e.g. 480x320, etc)
 *  + Hardware interfaces (e.g. parallel, SPI)
 *  + MCU customization (e.g. Teensy)
 *  + Font support (GFX, ILI9341, Adafruit 5x7, etc)
 * While the isolation is not complete (fonts are a notable weakness), the task of switching
 * libraries is largely confined to changes in just this class.
 *
 * The original implementation of AGraphicsDriver was for a 480x320 TFT display using the
 * HX8357D controller attached to a Teensy 4.1 through an SPI bus.
 *
 * DESIGN NOTES
 *  + The driver should probably be a singleton if multiple displays are forbidden
 *
 * DEPENDENCIES
 *  + https://github.com/adafruit/Adafruit_HX8357_Library
 *                          or
 *  + https://github.com/mjs513/HX8357_t3n
 *                          and
 *  + https://github.com/adafruit/Adafruit-GFX-Library
 **/

#include "AGraphicsDriver.h"

#include "Adafruit_GFX.h"  //HX8357_t3n requires you #include GFX before...
#include "DEBUG.h"         //For printf-style debugging on a Teensy sans JTAG :(
#include "HX8357_t3n.h"    //you #include the HX8357 variation.
#include "ft8_font.h"

HX8357_t3n* AGraphicsDriver::gfx;

//-----------------------------------------------------------------------------
//  Initialization
//-----------------------------------------------------------------------------

/**
 * @brief Bind the graphics driver to the HX8357_t3n version of the Adafruit GFX library
 * @param gfx Pointer to an object accessing the Adafruit Graphics Library
 *
 * You must invoke begin() prior to any other method in the graphics driver
 */
void AGraphicsDriver::begin(HX8357_t3n* gfx) {
    // Record the address of the object accessing Adafruit's graphics library
    this->gfx = gfx;

    // Setup the default font for the widgets.  Given the memory requirements for each font,
    // we're assuming most applications will use just this one font which you can
    // change here.  Alternatively, you can select other fonts with setFont().
    txtFont = &FT8Font;           // Record the default font
    this->gfx->setFont(txtFont);  // Setup display for this font
}

//-----------------------------------------------------------------------------
//  Graphical methods
//-----------------------------------------------------------------------------

/**
 * @brief Define the display's clip rectangle
 * @param clipX x-Screen coordinate
 * @param clipY y-Screen coordinate
 * @param clipW width in pixels
 * @param clipH height in pixels
 *
 * @note GFX supports a single clip window
 */
void AGraphicsDriver::setClipRect(ACoord clipX, ACoord clipY, ACoord clipW, ACoord clipH) {
    gfx->setClipRect(clipX, clipY, clipW, clipH);
}

/**
 * @brief Set the clip window to the full-screen
 */
void AGraphicsDriver::setClipRect() {
    gfx->setClipRect();
}

/**
 * @brief Fill a rectangle on the screen with the specified color
 * @param xCoord upper-left corner of rectangle
 * @param yCoord upper-left corner of rectangle
 * @param w width of rectangle
 * @param h height of rectangle
 * @param color Specified color
 */
void AGraphicsDriver::fillRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, AColor color) {
    gfx->fillRect(xCoord, yCoord, w, h, color);
}

/**
 * @brief Draw outline of a rectangle on the screen with the specified color
 * @param xCoord upper-left corner of rectangle
 * @param yCoord upper-left corner of rectangle
 * @param w width of rectangle
 * @param h height of rectangle
 * @param color Specified color
 */
void AGraphicsDriver::drawRect(ACoord xCoord, ACoord yCoord, ACoord w, ACoord h, AColor color) {
    gfx->drawRect(xCoord, yCoord, w, h, color);
}

//-----------------------------------------------------------------------------
//  Text methods
//-----------------------------------------------------------------------------

/**
 * @brief Select the specified GFX font
 * @param f Pointer to the GFXfont struct
 */
void AGraphicsDriver::setFont(const GFXfont* f) {
    gfx->setFont(f);
}

/**
 * @brief Select the specified ILI9341 font
 * @param f Pointer to an ILI9341_t3_font struct
 */
void AGraphicsDriver::setFont(const ILI9341_t3_font_t& f) {
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
ACoord AGraphicsDriver::getLeading() {
    return gfx->getLeading();
}

/**
 * @brief Place the text cursor at the specified coordinate
 * @param xCoord coordinate
 * @param yCoord coordinate
 */
void AGraphicsDriver::setCursor(ACoord xCoord, ACoord yCoord) {
    gfx->setCursor(xCoord, yCoord);
}

/**
 * @brief Set the text foreground color
 * @param fg Specified color
 *
 * @note Subsequent text will be displayed in the specified color
 */
void AGraphicsDriver::setTextColor(AColor fg) {
    gfx->setTextColor(fg);
}

/**
 * @brief Set the text foreground and background colors
 * @param fg Foreground color
 * @param bg Background color
 *
 * @note Subsequent text will be displayed in the specified colors
 */
void AGraphicsDriver::setTextColor(AColor fg, AColor bg) {
    gfx->setTextColor(fg, bg);
}

/**
 * @brief Specify whether text should wrap
 * @param w true==>wrap, false if not
 */
void AGraphicsDriver::setTextWrap(bool w) {
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
size_t AGraphicsDriver::writeText(const uint8_t* buffer, size_t size) {
    return gfx->write(buffer, size);
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
void AGraphicsDriver::getTextBounds(const uint8_t* buffer, uint16_t len, int16_t x, int16_t y,
                                    int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    gfx->getTextBounds(buffer, len, x, y, x1, y1, w, h);
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
void AGraphicsDriver::getTextBounds(const char* string, int16_t x, int16_t y,
                                    int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    gfx->getTextBounds(string, x, y, x1, y1, w, h);
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

void AGraphicsDriver::getTextBounds(const String& str, int16_t x, int16_t y,
                                    int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    gfx->getTextBounds(str, x, y, x1, y1, w, h);
}
