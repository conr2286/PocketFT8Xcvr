/**
 * @brief AToggleButton implements a GUI button with a toggled state
 *
 * Similar to many AGUI classes, the intended usage is for the application to inherit APixelBox
 * overriding its virtual methods.
 *
 */

#include "AToggleButton.h"

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"
#include "NODEBUG.h"
// #include "ft8_font.h"  //Include the default font

/**
 * @brief Construct AToggleButton object
 * @param str Pointer to the button's NUL-terminated char[] string label text
 * @param x1 Upper-left corner of button
 * @param y1 Upper-left corner of button
 * @param w Width
 * @param h Height
 *
 * Toggle buttons maintain a state variable toggled when touched.
 *
 * Warning:  Buttons save a copy of the pointer to their text label string, not the string
 * itself.  Applications must not unintentionally modify or destroy that char[] string as
 * it's used to repaint the button.
 */
AToggleButton::AToggleButton(const char *txt, ACoord x1, ACoord y1, ALength w, ALength h, int userDat, bool border) {
    // if (!Serial) Serial.begin(9600);
    // DPRINTF("str='%s', x1=%d, y1=%d, w=%d, h=%d\n", str, x1, y1, w, h);

    // Remember location and extent of the boundary box
    boundary.setCorners(x1, y1, w, h);

    // Some initialization of member variables
    str = String(txt);         // NUL-terminated char[] string pointer
    state = false;             // State
    this->userData = userDat;  // User-defined data

    // Eliminate the border if unwanted
    if (!border) {
        bgColor = ALT_BACKGROUND_COLOR;  // Make button visible w/o border
        bdColor = bgColor;               // Eliminate border
    }

    // Decorate the button
    AGUI::setClipRect(x1, y1, w, h);  // Set the clipping rectangle w/o consideration for rounded corners
    if (radius > 0) {
        AGUI::fillRoundRect(x1, y1, w, h, radius, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRoundRect(x1, y1, w, h, radius, bdColor);  // Draw rounder border if we need it
    } else {
        AGUI::fillRect(x1, y1, w, h, bgColor);                          // Erase background
        if (bdColor != bgColor) AGUI::drawRect(x1, y1, w, h, bdColor);  // Draw squared border if we need it
    }
    AGUI::setFont(font);                   // Use the widget's default font
    AGUI::setTextColor(fgColor, bgColor);  // Use the widget's default colors

    // Label the button
    AGUI::setTextWrap(false);                                                      // We don't wrap button text
    ACoord tx, ty, tw, th;                                                         // Lower-right coordinates, text width and text height
    AGUI::getTextBounds((uint8_t *)txt, strlen(txt), x1, y1, &tx, &ty, &tw, &th);  // Get the text bounds
    DPRINTF("x1=%d y1=%d tx=%d ty=%d tw=%d th=%d leading=%d\n", x1, y1, tx, ty, tw, th, AGUI::getLeading());
    AGUI::setCursor(boundary.x1 - 1 + (w - tw) / 2, boundary.y1 - 1 + (h - th) / 2);  // Center the text in the button
    AGUI::writeText((uint8_t *)txt, strlen(txt));                                     // Output text to button
    AGUI::setClipRect();
}  // AToggleButton

/**
 * @brief This is our private callback inherited from AWidget invoked when user clicks in this AToggleButton
 * @param xClick screen x-coord
 * @param yClick screen y-coord
 *
 * Note:  AWidget processTouch() notifies this method when the user clicks in this AToggleButton.
 */
void AToggleButton::onTouchWidget(ACoord xClick, ACoord yClick) {
    DTRACE();

    // Toggle this button's state and repaint it to reflect its toggled state
    state = !state;
    onRepaintWidget();

    // Notify the user-supplied callback of the selected item
    onTouchButton(userData);
}  // touchWIdget()

/**
 * @brief Repaint this toggle button
 *
 * The background color of repainted toggle buttons reflects their state.
 *
 * Note:  Buttons store a const pointer to their text strings (which the app should not modify)
 * referenced here when they are repainted.
 */
void AToggleButton::onRepaintWidget() {
    DTRACE();
    // The toggle button's current state determines its colors
    AColor bgCurrent = bgColor;  // Our default background
    AColor fgCurrent = fgColor;  // Our default foreground
    if (state) {
        bgCurrent = DEFAULT_SPECIAL_COLOR;  // On-state background color
        fgCurrent = AColor::A_BLACK;        // On-state foreground
    }

    // Get the button's boundaries
    ACoord w = boundary.x2 - boundary.x1 + 1;  // Width
    ACoord h = boundary.y2 - boundary.y1 + 0;  // Height

    // Re-decorate the button
    AGUI::setClipRect(boundary.x1, boundary.y1, w, h);                                             // Set the clipping rectangle w/o consideration for rounded corners
    AGUI::fillRoundRect(boundary.x1, boundary.y1, w, h, radius, bgCurrent);                        // Erase background
    if (bdColor != bgColor) AGUI::drawRoundRect(boundary.x1, boundary.y1, w, h, radius, bdColor);  // Draw border if we need it
    AGUI::setFont(font);                                                                           // Use the widget's default font
    AGUI::setTextColor(fgCurrent, bgCurrent);                                                      // Use toggle button's current state colors

    // Re-label the button
    ACoord tx, ty, tw, th;                                                            // Lower-right coordinates, text width and text height
    AGUI::getTextBounds(str, boundary.x1, boundary.y1, &tx, &ty, &tw, &th);           // Get the text bounds
    AGUI::setCursor(boundary.x1 - 1 + (w - tw) / 2, boundary.y1 - 1 + (h - th) / 2);  // Center the text in the button
    AGUI::writeText(str);                                                             // Output text to button
    AGUI::setClipRect();                                                              // Restore clip default
}  // onRepaintWidget()

/**
 * @brief Retrieve button state (i.e. on/off)
 * @return state
 */
bool AToggleButton::getState() const {
    return state;
}  // getState()

/**
 * @brief Set new state (i.e. on/off) value
 * @param newState
 */
void AToggleButton::setState(bool newState) {
    state = newState;
}  // setState()

/**
 * @brief Retrieve the value of userData
 * @return userData
 */
int AToggleButton::getUserData() const {
    return userData;
}  // getUserData()

/**
 * @brief Reset partial button state
 *
 * Resets state, enabled, colors, but not text str nor user data.  Repaints
 * the button.
 */
void AToggleButton::reset() {
    state = false;
    enabled = true;
    fgColor = AGUI::fgColor;
    bgColor = AGUI::bgColor;
    onRepaintWidget();
}  // reset()