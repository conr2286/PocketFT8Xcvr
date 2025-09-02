/**
 * @brief APixelBox is akin to an interactive AListBox but for bitmap pixels rather than list items
 *
 * APixelBox implements methods for displaying bitmap pixels within AWidget, and
 * interactively processing touch events on those pixels.
 *
 * Method parameters in this class using screen coordinates refer to them as xScreen/yScreen of type
 * ACoord while those referring to pixel positions within APixelBox refer to y/x of type APixelPos.
 *
 * Similar to many AGUI classes, the intended usage is for the application to inherit APixelBox
 * overriding its virtual methods.
 *
 * LIMITATION:  APixelBox does not have a repaint() method and does not cache the displayed pixels
 * for repainting.  Consequently, APixelBox is unable to repaint itself after a segment of its
 * region has been uncovered by another GUI control.
 *
 */

#include "APixelBox.h"

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"
#include "NODEBUG.h"

/**
 * @brief Constuctor for APixelBox using AWidget defaults
 * @param x1 Screen coordinate of upper-left corner
 * @param y1 Screen coordinate of upper-left corner
 * @param nRows Number of bitmap pixel rows
 * @param nCols Number of bitmap pixel columns
 *
 * @note Unlike other AGUI widgets, the width and height of APixelBox is expressed
 * in terms of the number of rows and columns of the bitmap that must fit within
 * the box (to ensure space for the application's image).
 *
 */
APixelBox::APixelBox(ACoord x1, ACoord y1, APixelPos nRows, APixelPos nCols) {
    // Calculate coordinates for the bitmap
    ACoord xBitMap = x1 + radius / 2 + 1;
    ACoord yBitMap = y1 + radius / 2 + 1;
    ACoord wBitMap = nCols - radius / 2 - 1;
    ACoord hBitMap = nRows - radius / 2 - 1;

    // Calculate width/height for the rounded boundary box
    ACoord w = nCols + radius;  // Width of box
    ACoord h = nRows + radius;  // Height of box

    // Decorate the box using AWidget defaults.  If AWidget's radius is 0, then we decorate
    // a squared rectangle, else a rounded rectangle.
    AGUI::setClipRect();                    // Clear any existing clip
    AGUI::fillRect(x1, y1, w, h, bgColor);  // Erase everything within boundary box
    if (radius > 0) {
        AGUI::drawRoundRect(x1, y1, w, h, radius, bdColor);  // Box has rounded corners
    } else {
        AGUI::drawRect(x1, y1, w, h, bdColor);  // Box has squared corners
    }

    // Remember location and extent of the boundary box and the enclosed bitmap
    boundary.setCorners(x1, y1, w, h);
    bitmap.setCorners(xBitMap, yBitMap, wBitMap, hBitMap);
}

/**
 * @brief Draw a single pixel at the specified position and color in APixelBox
 * @param x Pixel row
 * @param y Pixel col
 * @param color Pixel color
 *
 * The y and column positions are relative to the location of the APixelBox.  Thus,
 * position (0,0) refers to the upper-left *visible* pixel of the box (inside of the
 * border, if any;).
 */
void APixelBox::drawPixel(APixelPos x, APixelPos y, AColor color) const {
    // Setup the clip rectangle, draw, then restore clip
    AGUI::setClipRect(bitmap.x1, bitmap.y1, bitmap.w, bitmap.h);
    AGUI::drawPixel(bitmap.x1 + x, bitmap.y1 + y, color);
    AGUI::setClipRect();
}

/**
 * @brief Override notified when this widget is touched
 * @param xScreen Screen touch coordinate
 * @param yScreen Screen touch coordinate
 *
 * Our job is to determine which pixel was touched and notify the application
 *
 * @note The touch coordinate passed to application's onTouchPixel are the pixel coordinates
 * inside the box (relative to bitmap location), not the screen coordinates.
 */
void APixelBox::onTouchWidget(ACoord xScreen, ACoord yScreen) {
    DTRACE();

    // Calculate pixel coordinates within the bitmap.  Warning:  either position could be
    // negative if touch lies between bitmap and widget boundary.
    int xPos = xScreen - bitmap.x1;
    int yPos = yScreen - bitmap.y1;

    // Ignore touch within widget but outside the bitmap
    if ((xPos < 0) || (xPos > bitmap.x2)) return;
    if ((yPos < 0) || (yPos > bitmap.y2)) return;

    DPRINTF("Invoking onTouchPixel(%u %u)\n", xPos, yPos);
    onTouchPixel(xPos, yPos);
}
