/**
 * @brief AScrollBox is an interactive widget displaying scrolling lines of text
 *
 * New items (lines of text) are added to AScrollBox starting at the top, and then
 * downward.  Long lines are clipped; each item occupies a single line of text.
 * When a newly added item won't fit at the bottom of the box, existing lines are
 * scrolled up to make room for the new item at the bottom.
 *
 * DESIGN:  Why isn't repaint(item) a member of AScrollBoxItem?  Much of the data
 * required to repaint an item seems to belong to the AScrollBox containing the
 * item. If this changes, then consider refactoring repaint.
 *
 */

#include "AScrollBox.h"

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"
#include "DEBUG.h"

/**
 * @brief Build an item for AScrollBox
 * @param pContainer Pointer to AScrollBox containing this item
 * @param s Text String
 * @param fg Foreground color
 * @param bg Background color
 *
 * @note The text string, s, must not contain a NL character
 */
AScrollBoxItem::AScrollBoxItem(String& s, AColor fg, AColor bg, AScrollBox* pBox) {
    DPRINTF("AScrollBoxItem('%s')\n", s.c_str());
    str = s;                    // Item's text String
    str.replace('\n', ' ');     // We really can't tolerate NL chars in the String
    fgColor = fg;               // Item's foreground color
    bgColor = bg;               // Item's background color
    selected = false;           // Item is not selected
    scrollBoxContainer = pBox;  // Backlink from item to Scroll box container
}  // AScrollBoxItem()

void AScrollBoxItem::setItemColors(AColor fg, AColor bg) {
    DTRACE();
    if (scrollBoxContainer == nullptr) return;
    fgColor = fg;
    bgColor = bg;
    scrollBoxContainer->repaint(this);
    timeStamp = millis();  // Refresh timestamp
}

void AScrollBoxItem::setItemText(String s, AColor fg) {
    DPRINTF("setItemText(%s)\n", s.c_str());
    if (scrollBoxContainer == nullptr) return;
    fgColor = fg;
    str = s;  // str is AScrollBoxItem member object on the heap
    scrollBoxContainer->repaint(this);
    timeStamp = millis();  // Refresh timestamp
}

/**
 * @brief Build AScrollBox with the specified location, extent and default colors
 * @param x Upper-left corner
 * @param y Upper-left corner
 * @param w Width
 * @param h Height
 */
AScrollBox::AScrollBox(ACoord x, ACoord y, ALength w, ALength h, AColor bdColor) {
    // Initialize the member variables
    boundary.setCorners(x, y, w, h);  // Our boundary box
    leading = AGUI::getLeading();     // Get the leading (in pixels) for our font
    nDisplayedItems = 0;              // There are no items in this AScrollBox
    this->bdColor = bdColor;          // Border color

    // Reset the items[] arrays
    for (int i = 0; i < maxItems; i++) {
        displayedItems[i] = nullptr;  // No items yet
    }

    // Draw the empty box
    onRepaintWidget();

}  // AScrollBox()

/**
 * @brief Add an item to the bottom of this AListBox
 * @param pAScrollBox Pointer to scroll box container for this item
 * @param str The item's text String
 * @param fg Text color
 * @return Pointer to the newly added item
 *
 */
AScrollBoxItem* AScrollBox::addItem(AScrollBox* pAScrollBox, String str, AColor fg) {
    DPRINTF("str='%s'\n", str.c_str());

    // Too many items for our simple data structs?
    if (nDisplayedItems >= maxItems) return nullptr;

    // Build the new Item
    AScrollBoxItem* pNewItem = new AScrollBoxItem(str, fg, bgColor, pAScrollBox);  // Build item using widget's default colors
    pNewItem->timeStamp = millis();                                                // Record timestamp when item created

    // Scroll the displayed items up if the added item won't fit within widget's boundary box
    if (!itemWillFit(nDisplayedItems + 1)) {
        DPRINTF("Scroll with nDisplayedItems=%d\n", nDisplayedItems);
        scrollUpOneLine();  // Scrolling reduces nDisplayedItems by one, making room for new item
    }

    // Record the new item
    nDisplayedItems++;                       // Bump count of displayed items
    int newItemIndex = nDisplayedItems - 1;  // Calculate index into items[] where new item will reside
    displayedItems[newItemIndex] = pNewItem;

    // Paint the new item
    repaint(pNewItem);

    // Return pointer to the new item
    return pNewItem;
}  // addItem()

/**
 * @brief Repaint the entire AScrollBox
 *
 * Erases our widget's screen area and then repaints each item individually
 */
void AScrollBox::onRepaintWidget() {
    DTRACE();

    // Paint the entire background first, erasing whatever gibberish preceded us
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);        // Configure clip window to our boundary
    AGUI::fillRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bgColor);  // Erase everything within boundary box

    // Paint the boundary box.  Our AWidget determines if corners are rounded.
    if (radius > 0) {
        AGUI::drawRoundRect(boundary.x1, boundary.y1, boundary.w, boundary.h, radius, bdColor);  // Draw boundary Box  rounded corners
    } else {
        AGUI::drawRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bdColor);  // Draw boundary box  squared corners
    }

    // Paint the list of items
    for (int i = 0; i < nDisplayedItems; i++) {
        repaint(i);  // Repaint item indexed by i
    }
    AGUI::setClipRect();
}  // repaint()

/**
 * @brief Repaint item at the specified position index
 * @param index The specified item index
 * @param erase true to also erase existing text at this item
 * @return Index of repainted item or -1 if error
 *
 * @note Change the colors, if desired, before calling repaint()
 */
int AScrollBox::repaint(int index) {
    DTRACE();

    // Sanity checks
    if ((index < 0) || (index >= maxItems)) return -1;  // Bad index?
    if (displayedItems[index] == NULL) return -1;       // Non-existant item?

    // Repaint item
    repaint(displayedItems[index]);
    return index;
}  // repaint()

/**
 * @brief Repaint the specified item given its pointer
 * @param pItem Pointer to the item
 * @return Pointer to the item or nullptr if error
 */
AScrollBoxItem* AScrollBox::repaint(AScrollBoxItem* pItem) {
    DTRACE();
    // Sanity checks
    if (pItem == nullptr) return nullptr;

    // Configure app for writing text in this widget
    AGUI::setFont(font);                                                  // Use the widget's font
    AGUI::setTextColor(pItem->fgColor, pItem->bgColor);                   // Use the item's colors
    AGUI::setTextWrap(false);                                             // No wrapping, we clip 'em
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);  // Widget's clip rectangle... see, told ya

    // Find the item's index (i.e. what line is it on?)
    int index = getItemIndex(pItem);
    if (index < 0) return nullptr;

    // Calculate where to place the item
    int x1 = boundary.x1 + xOffset;
    int y1 = boundary.y1 + index * leading + yOffset;

    // Erase existing text in this item's location
    AGUI::fillRect(x1, y1, boundary.w - 2 * xOffset, leading, bgColor);

    // Write the item's text to display
    AGUI::setCursor(x1, y1);  // Text position
    DPRINTF("writeText(%s)\n", pItem->str.c_str());
    AGUI::writeText(pItem->str);  // Output text
    AGUI::setClipRect();          // Restore clip

    return pItem;
}  // repaint()

/**
 * @brief Determine if nItems could fit in the boundary box
 * @param nItems #items
 * @return true if they fit, else false
 */
bool AScrollBox::itemWillFit(int nItems) const {
    // DTRACE();

    // Does the box have room for anything at all?
    int nLinesAvailable = (boundary.h - 2 * yOffset) / leading;  // Leading is pixel distance from top of one line to next
    if (nLinesAvailable <= 0) return false;                      // No

    // Do count items fit within the box?
    if (nItems <= nLinesAvailable) return true;  // Yes
    return false;                                // No
}  // itemWillFit()

/**
 * @brief Scroll all of the items up a text line, leaving one blank line at the bottom of the boundary box
 *
 * @note Depending upon the capabilities of the underlying display system, this can be accomplished
 * either by redrawing all but the top line, or by having the hardware perform a real scroll.  No matter
 * how accomplished, the top line will be removed from items[] and the bottom line vanishes.
 */
void AScrollBox::scrollUpOneLine() {
    DTRACE();

    // Calculate pixel locations of the text to be scrolled
    ACoord x = boundary.x1 + xOffset;      // Upper left corner of scrolled region
    ACoord y = boundary.y1 + yOffset;      // Upper left corner of scrolled region
    ALength w = boundary.w - 2 * xOffset;  // Width
    ALength h = boundary.h - 2 * yOffset;  // Height

    // Configure scrolling parameters
    AGUI::enableScroll();                       // TODO:  Is this really necessary?
    AGUI::resetScrollBackgroundColor(bgColor);  // Config color of empty space created at bottom of scroll rectangle
    AGUI::setScrollTextArea(x, y, w, h);        // Configure location/extent of scroll rectangle

    // Scroll the display
    AGUI::scrollTextArea(leading);  // Scroll-up one line of pixels
    AGUI::disableScroll();          // TODO:  Not sure we need to enable/disable

    // Update displayItems[] to reflect how the items moved up a line
    if (nDisplayedItems <= 0) return;  // Sanity check
    delete displayedItems[0];          // The top item is "gone" (scrolled off the top)
    for (int i = 1; i < nDisplayedItems; i++) {
        displayedItems[i - 1] = displayedItems[i];  // Move remaining items up one line
    }
    displayedItems[nDisplayedItems - 1] = nullptr;  // The bottom line moved up

    // Update count of displayed items
    nDisplayedItems--;
    DPRINTF("After scrolling, nDisplayedItems = %d\n", nDisplayedItems);
    delay(2000);

}  // scrollUpOneLine()

/**
 * @brief Set an item's foreground and background colors
 * @param pItem Identifies the item
 * @param fgColor New foreground color
 * @param bgColor New background color
 * @return Pointer to modified item or nullptr if error
 */
AScrollBoxItem* AScrollBox::setItemColors(AScrollBoxItem* pItem, AColor fgColor, AColor bgColor) {
    DTRACE();

    // Sanity check
    if (pItem == nullptr) return nullptr;

    // Reconfigure and repaint the item
    pItem->fgColor = fgColor;
    pItem->bgColor = bgColor;
    repaint(pItem);
    return pItem;
}

/**
 * @brief Get the index of the specified item
 * @param pItem Pointer to item
 * @return Item's index into items[] array or -1 if not found
 */
int AScrollBox::getItemIndex(AScrollBoxItem* pItem) const {
    // DTRACE();
    //  Sanity check
    if (pItem == nullptr) return -1;

    // Find it
    for (int index = 0; index < maxItems; index++) {
        // DPRINTF("index=%d\n", index);
        if (pItem == displayedItems[index]) return index;
    }
    return -1;
}

/**
 * @brief Get count of displayed items
 * @return count
 */
int AScrollBox::getCount() const {
    return nDisplayedItems;
}

/**
 * @brief Reset this AScrollBox
 *
 * All items are removed and an empty box is displayed
 */
void AScrollBox::reset() {
    // Remove all the items
    for (int i = 0; i < nDisplayedItems; i++) {
        removeItem(i);
    }

    // Update members
    nDisplayedItems = 0;

    // Repaint this box
    onRepaintWidget();
}  // reset()

/**
 * @brief Destructor erases item from display and frees the data structs
 */
AScrollBox::~AScrollBox() {
    DTRACE();

    // Also erase the widget's border
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);        // Configure clip window to our boundary
    AGUI::fillRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bgColor);  // Erase everything within boundary box
    AGUI::setClipRect();                                                        // Restore default clip

    // Purge the items
    reset();
}

/**
 * @brief Removes the specified item from the AScrollBox data structures
 * @param index Specifies which item
 * @return index of removed item or -1 if error
 *
 * @note Does not update the display.
 *
 * @note WARNING:  nullptr replaces the removed AScrollBoxItem pointer
 *
 */
int AScrollBox::removeItem(int index) {
    DTRACE();

    // Sanity checks
    if ((index < 0) || (index >= nDisplayedItems)) return -1;
    if (displayedItems[index] == nullptr) return -1;

    // Delete the item and its reference in displayedItems[]
    delete displayedItems[index];
    displayedItems[index] = nullptr;
    return index;
}

/**
 * @brief Override AWidget::onTouchWidget to receive touch events for this AScrollBox
 * @param xTouch Screen coordinate of touch event
 * @param yTouch Screen coordinate of touch event
 *
 * @brief AWidget notifies us when a touch event occurs inside this AScrollBox
 *
 * @note The coordinates are those of the screen, not offsets within this AScrollBox
 */
void AScrollBox::onTouchWidget(ACoord xTouch, ACoord yTouch) {
    DTRACE();

    // Find the selected item
    AScrollBoxItem* item = getSelectedItem(xTouch, yTouch);  // Which item was touched?
    if (item == nullptr) return;                             // Not an item

    // Toggle selection state
    item->selected = !item->selected;

    // Notify application of touched item
    onTouchItem(item);  // Application overrides onTouchItem() to receive notifications of touch events for items
}

/**
 * @brief Get a pointer to the touched item
 * @param xClick Screen x-Coord
 * @param yClick Screen y-Coord
 * @return Pointer to the AScrollBoxItem or nullptr if none
 */
AScrollBoxItem* AScrollBox::getSelectedItem(ACoord xClick, ACoord yClick) const {
    DTRACE();

    // Perhaps this click lies entirely outside this widget's boundary
    if (!boundary.isWithin(xClick, yClick)) return nullptr;

    // Calculate index of clicked item
    unsigned index = (yClick - boundary.y1) / leading;
    if (index >= maxItems) return nullptr;  // Validate calculated index

    // Return pointer or null (empty indices of displayedItems[] are nullptr)
    return (displayedItems[index]);

}  // getSelectedItem()

/**
 * @brief Review top item timestamp and scroll up if ancient
 *
 * @note The timeout period is hardwired to 6 minutes
 *
 * We only remove the *top* item in the box, we don't check for holes
 */
void AScrollBox::reviewTimeStamps() {
    unsigned long now = millis();
    const unsigned long timeoutMillis = 6 * 60 * 1000UL;

    // Sanity checks
    if (displayedItems[0] == nullptr) return;

    // If top item has expired, scroll it off the box
    if ((now - displayedItems[0]->timeStamp) > timeoutMillis) {
        scrollUpOneLine();
    }
}

String* AScrollBoxItem::getItemText() {
    return &str;
}