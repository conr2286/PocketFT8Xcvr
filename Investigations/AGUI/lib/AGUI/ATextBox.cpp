/**
 * @brief ATextBox is a non-interactive widget for displaying text
 *
 * @note ATextBox stores a copy of the text string and can repaint itself
 *
 * @note Caution:  Avoid confusion with Java Swing --- ATextBox does not
 * permit the operator to interactively edit the text.  That would be an
 * ATextEditor if implemented in AGUI.
 */

#include "ATextBox.h"

#include <Arduino.h>

#include <string>

#include "AGUI.h"
#include "AWidget.h"
#include "DEBUG.h"

/**
 * @brief Build and display ATextBox object
 * @param txt The text to display
 * @param x Upper-left coordinate
 * @param y Upper-left coordinate
 * @param w Width
 * @param h Height
 */
ATextBox::ATextBox(const char *txt, ACoord x, ACoord y, ALength w, ALength h, bool border) {
    DPRINTF("txt='%s' x=%d y=%d w=%d h=%d\n", txt, x, y, w, h);

    // Initialize member variables
    boundary.setCorners(x, y, w, h);
    str = String(txt);

    // Eliminate the border if unwanted
    if (!border) {
        bdColor = bgColor;
    }

    // Setup clipping rectangle without regard for rounded vs. squared corners
    AGUI::setClipRect(x, y, w, h);

    // Decorate the text box
    if (radius > 0) {
        AGUI::fillRoundRect(x, y, w, h, radius, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRoundRect(x, y, w, h, radius, bdColor);  // Draw rounded border if we need it
    } else {
        AGUI::fillRect(x, y, w, h, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRect(x, y, w, h, bdColor);  // Draw squared border if we need it
    }

    // Setup the clip rectangle to inside the boundary.
    int16_t clipX = boundary.x1 + 1;
    int16_t clipY = boundary.y1 + 1;
    int16_t clipW = boundary.x2 - boundary.x1 - 1;  // Clip width sans border
    int16_t clipH = boundary.y2 - boundary.y1 - 1;  // Clip height sans border

    // Now adjust the clip to allow room for the border if we have one
    if (hasBorder()) {  // Does list box have a border?
        clipW -= 2;     // Width descreases to make room for border
        clipH -= 2;     // As does height to make room for border
    }

    // Configure app for writing the text
    AGUI::setFont(defaultFont);            // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);  // Use the widget's default colors
    AGUI::setTextWrap(true);               // We do wrap text
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Write the text
    int16_t drawX = clipX + 1;  // X-coord
    int16_t drawY = clipY + 1;  // Y-coord
    AGUI::setCursor(drawX, drawY);
    AGUI::writeText((uint8_t *)txt, strlen(txt));  // Output text to box
}


void ATextBox::repaintWidget(void) {
    // Setup clipping rectangle without regard for rounded vs. squared corners
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);

    // Decorate the text box
    if (radius > 0) {
        AGUI::fillRoundRect(boundary.x1, boundary.y1, boundary.w, boundary.h, radius, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRoundRect(boundary.x1, boundary.y1, boundary.w, boundary.h, radius, bdColor);  // Draw rounded border if we need it
    } else {
        AGUI::fillRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bdColor);  // Draw squared border if we need it
    }

    // Setup the clip rectangle to inside the boundary.
    int16_t clipX = boundary.x1 + 1;
    int16_t clipY = boundary.y1 + 1;
    int16_t clipW = boundary.x2 - boundary.x1 - 1;  // Clip width sans border
    int16_t clipH = boundary.y2 - boundary.y1 - 1;  // Clip height sans border

    // Now adjust the clip to allow room for the border if we have one
    if (hasBorder()) {  // Does list box have a border?
        clipW -= 2;     // Width descreases to make room for border
        clipH -= 2;     // As does height to make room for border
    }

    // Configure app for writing the text
    AGUI::setFont(defaultFont);            // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);  // Use the widget's default colors
    AGUI::setTextWrap(true);               // We do wrap text
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Write the text
    int16_t drawX = clipX + 1;  // X-coord
    int16_t drawY = clipY + 1;  // Y-coord
    AGUI::setCursor(drawX, drawY);
    AGUI::writeText(str);  // Output text to box

}  // Derived classes must overide doRepaintWidget() to repaint themselves
