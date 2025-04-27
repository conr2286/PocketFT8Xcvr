/**
 * @brief AListBox is an interactive widget displaying scrolling lines of text
 *
 * New items (lines of text) are added to AListBox starting at the top, and then
 * downward.  Long lines are clipped; each item occupies a single line of text.
 * When a newly added item won't fit at the bottom of the box, existing lines are
 * scrolled up to make room for the new item at the bottom.
 *
 * DESIGN:  Why isn't repaint(item) a member of AListBoxItem?  Much of the data
 * required to repaint an item seems to belong to the AListBox containing the
 * item. If this changes, then consider refactoring repaint.
 *
 */

#include "AListBox.h"

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"
#include "NODEBUG.h"

/**
 * @brief Build an item for AListBox
 * @param pContainer Pointer to AListBox containing this item
 * @param s Text String
 * @param fg Foreground color
 * @param bg Background color
 *
 * @note The text string, s, must not contain a NL character
 */
AListBoxItem::AListBoxItem(String s, AColor fg, AColor bg, AListBox* pBox) {
    // if (!Serial) Serial.begin(9600);
    // Serial.println("AListBoxItem()");
    str = s;                  // Copy Item's text String
    str.replace('\n', ' ');   // We really can't tolerate NL chars in the String
    fgColor = fg;             // Item's foreground color
    bgColor = bg;             // Item's background color
    selected = false;         // Item is not selected
    listBoxContainer = pBox;  // Backlink from item to Scroll box container
}  // AListBoxItem()

/**
 * @brief Set an items fg/bg colors
 * @param fg Foreground color
 * @param bg Background color
 */
void AListBoxItem::setItemColors(AColor fg, AColor bg) {
    DTRACE();
    fgColor = fg;
    bgColor = bg;
    listBoxContainer->repaint(this);
}

/**
 * @brief Change an item's text
 * @param s New String
 * @param fg Color
 */
void AListBoxItem::setItemText(const String& s, AColor fg) {
    DTRACE();
    fgColor = fg;
    str = s;
    listBoxContainer->repaint(this);
}

/**
 * @brief Build AListBox with the specified location, extent and default colors
 * @param x Upper-left corner
 * @param y Upper-left corner
 * @param w Width
 * @param h Height
 * @param bdColor Border color (default is black)
 */
AListBox::AListBox(ACoord x, ACoord y, ALength w, ALength h, AColor bdColor) {
    // if (!Serial) Serial.begin(9600);
    // Serial.println("AListBox()");

    // Initialize the member variables
    boundary.setCorners(x, y, w, h);  // Our boundary box
    leading = AGUI::getLeading();     // Get the leading (in pixels) for application's font
    nDisplayedItems = 0;              // There are no items in this AListBox at this time
    this->bdColor = bdColor;          // Border color

    // Reset the items[] arrays
    for (int i = 0; i < maxItems; i++) {
        displayedItems[i] = nullptr;  // No items yet
    }

    // Draw the empty box
    onRepaintWidget();

}  // AListBox()

/**
 * @brief Add a new item to the bottom of this AListBox
 * @param pListBox Pointer to list box container for this item
 * @param str The item's text String
 * @param fg Text color
 * @return Pointer to the newly added item
 *
 * No scrolling in a list box; that which doesn't fit clips
 *
 */
AListBoxItem* AListBox::addItem(AListBox* pListBox, const String str, AColor fg) {
    DPRINTF("str='%s'\n", str.c_str());

    // Too many items for our primitive data structs?
    if (nDisplayedItems >= maxItems) return nullptr;

    // Build the new Item
    AListBoxItem* pNewItem = new AListBoxItem(str, fg, bgColor, pListBox);  // Build item using widget's default colors

    // Record the new item
    int newItemIndex = nDisplayedItems;       // Savev index into displayedItems[] where new item will reside
    nDisplayedItems++;                        // Bump count of displayed items
    displayedItems[newItemIndex] = pNewItem;  // Record new item at bottom of box

    // Paint the new item
    repaint(pNewItem);

    // Return pointer to the new item
    return pNewItem;
}  // addItem()

/**
 * @brief Create an item at specified index
 * @param index New item's index
 * @param str New item's text String
 * @param fg New item's fgColor
 * @return index or -1 if error
 *
 * @note Creating a new item can create "holes" (indices without items) in the list
 */
int AListBox::setItem(int index, const String& str, AColor fg, AColor bg) {
    // Sanity checks
    if ((index < 0) || (index >= maxItems)) return -1;  // Bad index?

    // Perhaps we are replacing an existing item?
    if (displayedItems[index] != NULL) {
        removeItem(index);  // Yes, remove existing item
    }

    // Build the new Item
    AListBoxItem* pNewItem = new AListBoxItem(str, fg, bg, this);  // Build item using widget's default background color

    // Record the new item
    displayedItems[index] = pNewItem;                           // Record new item somewhere in this box
    if (index >= nDisplayedItems) nDisplayedItems = index + 1;  // Bump count of items&holes if new item below existing items

    // Paint the new item
    repaint(pNewItem);

    // Return pointer to the new item
    return index;
}

/**
 * @brief Repaint the entire AListBox
 *
 * Erases our widget's panel then repaints each item individually
 */
void AListBox::onRepaintWidget() {
    DTRACE();

    // Paint the entire panel background first, erasing whatever gibberish preceded us
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);        // Configure clip window to our boundary
    AGUI::fillRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bgColor);  // Erase everything within boundary box

    // Paint the boundary box
    if (radius > 0) {
        AGUI::drawRoundRect(boundary.x1, boundary.y1, boundary.w, boundary.h, radius, bdColor);  // Draw boundary Box  rounded corners
    } else {
        AGUI::drawRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bdColor);  // Draw boundary box  squared corners
    }

    // Paint the list of items
    for (int i = 0; i < nDisplayedItems; i++) {
        repaint(i);  // Repaint item indexed by i
    }
    AGUI::setClipRect();  // Restore clip window to default
}  // repaint()

/**
 * @brief Repaint item at the specified index
 * @param index The specified item index
 * @return Index of repainted item or -1 if error
 *
 * @note Change the colors, if desired, before calling repaint()
 */
int AListBox::repaint(int index) {
    DTRACE();

    // Sanity checks
    if ((index < 0) || (index >= maxItems)) return -1;  // Bad index?
    if (displayedItems[index] == NULL) return -1;       // Non-existant item?

    // Repaint item
    repaint(displayedItems[index]);
    return index;
}  // repaint()

/**
 * @brief Repaint the specified item
 * @param pItem Pointer to item to repaint
 * @return pointer to item or nullptr if error
 */
AListBoxItem* AListBox::repaint(AListBoxItem* pItem) const {
    // if (!Serial) Serial.begin(9600);

    // Sanity checks
    if (pItem == nullptr) return nullptr;

    // Map item pointer to its index into displayedItems[]
    int index = getItemIndex(pItem);
    if (index < 0) return nullptr;

    // Configure app for writing text in this widget
    AGUI::setFont(defaultFont);                                           // Use the widget's font
    AGUI::setTextColor(pItem->fgColor, pItem->bgColor);                   // Use the item's colors
    AGUI::setTextWrap(false);                                             // No wrapping, we clip 'em
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);  // Widget's clip rectangle

    // Calculate where to place the item
    int x1 = boundary.x1 + xOffset;
    int y1 = boundary.y1 + index * leading + yOffset;

    // Erase existing text in this item's location
    AGUI::fillRect(x1, y1, boundary.w - 2 * xOffset, leading, bgColor);

    // Write the item's text to display
    AGUI::setCursor(x1, y1);      // Text position
    AGUI::writeText(pItem->str);  // Output the text
    AGUI::setClipRect();          // Restore default clip

    return pItem;
}  // repaint()

/**
 * @brief Set item's foreground color
 * @param index Specified item
 * @param fg Color
 * @param bg Color (default is black)
 * @return Index or -1 if error
 */
int AListBox::setItemColor(int index, AColor fg, AColor bg) {
    // Sanity checks
    if ((index < 0) || (index >= maxItems) || (index >= nDisplayedItems)) return -1;

    // Lookup item and change it
    if (setItemColors(displayedItems[index], fg, bg) == nullptr) return -1;
    return index;
}

/**
 * @brief Set an item's foreground and background colors
 * @param pItem Identifies the item
 * @param fgColor New foreground color
 * @param bgColor New background color
 * @return Pointer to modified item or nullptr if error
 */
AListBoxItem* AListBox::setItemColors(AListBoxItem* pItem, AColor fg, AColor bg) {
    DTRACE();

    // Sanity check
    if (pItem == nullptr) return nullptr;

    // Reconfigure and repaint the item
    pItem->fgColor = fg;
    pItem->bgColor = bg;
    repaint(pItem);
    return pItem;
}

/**
 * @brief Get the index of the specified item
 * @param pItem Pointer to item
 * @return Item's index into items[] array or -1 if not found
 */
int AListBox::getItemIndex(const AListBoxItem* pItem) const {
    DTRACE();
    // Sanity check
    if (pItem == nullptr) return -1;

    // Find it
    for (int index = 0; index < maxItems; index++) {
        if (pItem == displayedItems[index]) return index;
    }
    return -1;
}

/**
 * @brief Get this item's index
 * @return index or -1 if error
 */
int AListBoxItem::getIndex() const {
    return listBoxContainer->getItemIndex(this);
}  // getIndex()

/**
 * @brief Get count of displayed items
 * @return count
 */
int AListBox::getCount() const {
    return nDisplayedItems;
}

/**
 * @brief Reset this AListBox
 *
 * All items are removed and an empty box is displayed
 */
void AListBox::reset() {
    // Remove all the items
    for (int i = 0; i < nDisplayedItems; i++) {
        removeItem(i);
    }

    // Update members
    nDisplayedItems = 0;

    // Repaint this box
    onRepaintWidget();
}  // reset()

AListBox::~AListBox() {
    DTRACE();

    // Purge the items
    reset();

    // Also erase the widget's border
    AGUI::setClipRect(boundary.x1, boundary.y1, boundary.w, boundary.h);        // Configure clip window to our boundary
    AGUI::fillRect(boundary.x1, boundary.y1, boundary.w, boundary.h, bgColor);  // Erase everything within boundary box
    AGUI::setClipRect();                                                        // Default clip window
}

/**
 * @brief Removes the specified item from the AListBox data structures
 * @param index Specifies which item
 * @return index of removed item or -1 if error
 *
 * @note Does not update the display, just cleans data structs
 *
 * @note WARNING:  nullptr replaces the removed AListBoxItem pointer
 *
 */
int AListBox::removeItem(int index) {
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
 * @brief Override AWidge to receive touch events for this AListBox
 * @param xTouch Screen coordinate of touch event
 * @param yTouch Screen coordinate of touch event
 *
 * @brief AWidget notifies us when a touch event occurs inside this AListBox
 *
 * @note The coordinates are those of the screen, not offsets within this AListBox
 */
void AListBox::onTouchWidget(ACoord xTouch, ACoord yTouch) {
    DTRACE();

    // Find the selected item
    AListBoxItem* item = getSelectedItem(xTouch, yTouch);  // Which item was touched?
    if (item == nullptr) return;                           // Not an item

    // Toggle selection state
    item->selected = !item->selected;

    // Notify application of touched item
    onTouchItem(item);  // Application overrides onTouchItem() to receive notifications of touch events for items
}

/**
 * @brief Get a pointer to the touched item
 * @param xClick Screen x-Coord
 * @param yClick Screen y-Coord
 * @return Pointer to the AListBoxItem or nullptr if none
 */
AListBoxItem* AListBox::getSelectedItem(ACoord xClick, ACoord yClick) const {
    DTRACE();

    // Perhaps this click lies entirely outside this widget's boundary
    if (!boundary.isWithin(xClick, yClick)) return nullptr;

    // Calculate index of clicked item
    unsigned index = (yClick - boundary.y1) / leading;
    if (index >= maxItems) return nullptr;  // Validate calculated index

    // Return pointer or null (empty indices of displayedItems[] are nullptr)
    return (displayedItems[index]);

}  // getSelectedItem()
