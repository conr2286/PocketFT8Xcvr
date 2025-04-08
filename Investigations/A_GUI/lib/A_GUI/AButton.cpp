

#include "AButton.h"

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"
#include "DEBUG.h"
#include "ft8_font.h"  //Include the default font


AButton::AButton(const char *str, ACoord x1, ACoord y1, ACoord w, ACoord h) {
    if (!Serial) Serial.begin(9600);
    DPRINTF("str='%s', x1=%d, y1=%d, w=%d, h=%d\n", str, x1, y1, w, h);

    // Remember location and extent of the boundary box
    boundary.setCorners(x1, y1, x1 + w, y1 + h);

    // Define a suitable radius for the corners
    r = 2;

    // Decorate the button
    AGUI::setClipRect(x1, y1, w, h);                                        // Set the clipping rectangle w/o consideration for rounded corners
    AGUI::fillRoundRect(x1, y1, w, h, r, bgColor);                          // Erase background
    if (bdColor != bgColor) AGUI::drawRoundRect(x1, y1, w, h, r, bdColor);  // Draw border if we need it
    AGUI::setFont(defaultFont);                                             // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);                                   // Use the widget's default colors

    // Label the button
    ACoord tx, ty, tw, th;                                                   // Lower-right coordinates, text width and text height
    AGUI::getTextBounds((uint8_t *)str, strlen(str), x1, y1, &tx, &ty, &tw, &th);  // Get the text bounds
    AGUI::setCursor(x1 + tw / 2, y1 + th / 2);                                     // Center the text in the button
    AGUI::writeText((uint8_t *)str, strlen(str));                                  // Output text to button
}
