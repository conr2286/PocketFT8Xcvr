/**
 * @brief ATextBox is a non-interactive widget for displaying text
 *
 * @note ATextBox stores a copy of the single text string and can repaint itself
 *
 * @note Caution:  Avoid confusion with Java Swing --- ATextBox does not
 * enable the operator to interactively edit the text.  That would be an
 * ATextEditor if implemented in AGUI.
 *
 * @note While the user should subclass most AGUI widgets and override
 * their virtual methods, there's really not much to override here.
 *
 */

#include "ATextBox.h"

#include <Arduino.h>

// #include <string>

#include "AGUI.h"
#include "AWidget.h"
#include "NODEBUG.h"

/**
 * @brief Build and display ATextBox object
 * @param txt The text to display
 * @param x Upper-left coordinate
 * @param y Upper-left coordinate
 * @param w Width
 * @param h Height
 */
ATextBox::ATextBox(const char *txt, ACoord x, ACoord y, ALength w, ALength h, AColor border) {
    DPRINTF("txt='%s' x=%d y=%d w=%d h=%d\n", txt, x, y, w, h);

    // Initialize member variables
    boundary.setCorners(x, y, w, h);
    str = String(txt);
    bdColor = border;

    // Setup clipping rectangle without regard for rounded vs. squared corners
    AGUI::setClipRect(x, y, w, h);

    // Decorate the text box whose AWidget might specify rounded corners
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
    AGUI::setFont(font);                   // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);  // Use the widget's default colors
    AGUI::setTextWrap(true);               // We do wrap text
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Write the text
    int16_t drawX = clipX + 1;  // X-coord
    int16_t drawY = clipY + 1;  // Y-coord
    AGUI::setCursor(drawX, drawY);
    AGUI::writeText((uint8_t *)txt, strlen(txt));  // Output text to box
    AGUI::setClipRect();
}  // ATextBox()

/**
 * @brief Override the AWidget repaint method
 *
 * @note Since we stored the text, we can repaint without help from application callback
 */
void ATextBox::onRepaintWidget(void) {
    DTRACE();
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
    AGUI::setFont(font);                   // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);  // Use the widget's default colors
    AGUI::setTextWrap(true);               // We do wrap text inside the clip
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Erase old text and paint new text
    int16_t drawX = clipX + 1;                                              // X-coord
    int16_t drawY = clipY + 1;                                              // Y-coord
    AGUI::fillRect(drawX, drawY, boundary.w - 2, boundary.h - 2, bgColor);  // Erase existing text
    AGUI::setCursor(drawX, drawY);                                          // Text drawing posiiton
    if (str != String("")) AGUI::writeText(str);                            // Output non-empty text to box
    AGUI::setClipRect();                                                    // Restore default clip

}  // onRepaintWidget()

/**
 * @brief Change the display text in this ATextBox
 * @param txt NUL-terminated char[] string
 *
 * @note Repaints the text box with the new text
 */
void ATextBox::setText(const char *txt, AColor fg) {
    fgColor = fg;
    str = String(txt);
    onRepaintWidget();
}

/**
 * @brief Change the display text in this ATextBox
 * @param txt String to display
 */
void ATextBox::setText(String &rstr, AColor fg) {
    fgColor = fg;
    // DPRINTF("setText %s\n", rstr.c_str());
    str = String(rstr);
    onRepaintWidget();
}

/**
 * @brief Reset this text box, clearing all text.  Colors remain unmodified.
 */
void ATextBox::reset() {
    str = "";  // Nothing to display
    onRepaintWidget();
}