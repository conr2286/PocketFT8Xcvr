/**
 * AListBox implements a simplified GUI ListBox widget
 *
 * FEATURES
 *  + Interactive GUI widget
 *  + Displays a list of items in a rectangular box
 *  + Each item is a line of text
 *  + Items are clipped to the box borders
 *  + Items are identified by integer indexes, 0..N
 *  + Optional visible border
 *  + Optional background color
 *  + Optional foreground color for each item
 *  + Item selections
 *
 * SIMPLIFICATIONS
 *  + AListBox does not support scrolling
 *  + AListBox does not support multiple selections
 *  + AListBox cannot be resized
 *  + Selections are polled (see getSelection() description)
 *  + The text font uses its widget's default
 *
 * USAGE
 *  + An application should create a derived class from AListBox overriding its virtual methods.
 *  + The derived class should implement the touchItem() method to handle touch events for its items.
 *
 * MISC
 * AListBox "is a" AWidget, and it implements the Print interface so you can print() to it.
 *
 * Items in AListBox are drawn with the widget's current font, size and color.
 *
 * @note Items are not widgets or even objects.
 *
 */

#include "AListBox.h"

#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
// #include <string.h>

#include "AGUI.h"
#include "AWidget.h"
#include "NODEBUG.h"

/**
 * @brief Build AListBox sans border line using width and height
 * @param x1 Upper-left corner pixel x-coord
 * @param y1 Upper-left corner pixel y-coord
 * @param w Width pixels
 * @param h Height pixels
 */
AListBox::AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h) {
    if (!Serial) Serial.begin(9600);
    DPRINTF("AListBox()=%08x\n", this);
    DPRINTF("x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);

    // Remember location and extent of the boundary box
    boundary.setCorners(x1, y1, w, h);
    // DTRACE();

    // Initialize some default text values for this new list box
    // DTRACE();
    leading = AGUI::getLeading();  // Get the leading (in pixels) for this font
    DTRACE();

    // Decorate the list box
    AGUI::setClipRect();  // Clear any existing clip
    DTRACE();
    AGUI::fillRoundRect(x1, y1, w, h, radius, bgColor);  // Background
    DTRACE();

    // No items exist yet and none are selected
    for (int i = 0; i < maxItems; i++) {
        itemPixelCount[i] = 0;
        itemSelected[i] = false;
        itemTxt[i] = NULL;
        itemColor[i] = bgColor;
    }

    // The first item added to the list will be item 0
    nextItem = 0;  // Index of where addItem() places unnumbered additions
    DTRACE();
}  // AListBox()

/**
 * @brief Build AListBox with a border line using width and height
 * @param x1 Upper-left corner pixel x-coord
 * @param y1 Upper-left corner pixel y-coord
 * @param w Width pixels
 * @param h Height pixels
 * @param bdColor The border line's color
 */
AListBox::AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h, AColor bdColor) : AListBox(x1, y1, w, h) {
    if (!Serial) Serial.begin(9600);
    DPRINTF("x1=%d, y1=%d, w=%d, h=%d, bdColor=0x%x\n", x1, y1, w, h, bdColor);

    // Initialize additional default values for this list box
    this->bdColor = bdColor;  // Border color

    // Decorate the list box border
    DPRINTF("Border:  x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);
    AGUI::drawRoundRect(x1, y1, w, h, radius, bdColor);  // Draw the border (might be just background)

    DTRACE();
}  // AListBox()

/**
 * @brief Add an item to the list box and increment nextItem
 * @param index Specifies which item in the list
 * @param txt NUL-terminated char[] string text to draw at the list item
 * @param fgColor Foreground color of the item's text
 * @return Index of added item or -1 if error
 *
 * An "item" is a single line of text in a list box.  The implementation is
 * limited to 24 items in a box.
 *
 * The char[] string should not contain a NL character.
 *
 * The specified item, if it already exists, will be overwritten.
 *
 * The text background color will be that of the AListBox instance.
 *
 * Text strings are written left..right and clipped at the list box's
 * righthand border.
 *
 * Advances the nextItem index to the following line.
 */
int AListBox::addItem(int index, const char *txt, AColor fgColor) {
    DPRINTF("addItem(%d,%p,%04x) bgColor=%04x", index, txt, fgColor, bgColor);

    // Display the item
    int result = setItem(index, txt, fgColor, bgColor);

    // Advance nextItem for subsequent additions
    if (result >= 0) nextItem++;  // This might increment beyond the AListBox clip rectangle
    return result;                // Return index of item just written

}  // addItem()

/**
 * @brief Places an item at index
 * @param index Specifies index of item
 * @param txt Nul-terminated string to place
 * @param fgColor Foreground color of the item's text
 * @param bgColor Background color of the item's text
 * @return index of item or -1 if error
 *
 * @note A successful return leaves nextItem = index.  Note that nextItem is not incremented
 *
 * @note The text must not contain a NL --- an item is a single line of text
 * @note setItem() will  *not* increment nextItem!!!
 *
 */
int AListBox::setItem(int index, const char *txt, AColor fgColor, AColor bgColor) {
    DPRINTF("setItem(%d, %s, %04x, %04x)\n ", index, txt, fgColor, bgColor);

    //  Sanity checks
    if ((index < 0) || (index >= maxItems)) return -1;
    if (txt == NULL) return -1;

    // Truncate text containing a NL --- An item is *one* line of text or at least will be soon!!!
    char *pNL = strchr(txt, '\n');  // TODO:  we really should loop to catch multiple NL chars
    if (pNL != NULL) *pNL = 0;      // Replace NL with a NUL

    // Remove an existing item, if any, at this index
    DTRACE();
    removeItem(index);

    // Create entries for this item
    nextItem = index;                  // Informs writeItem() where to write
    itemPixelCount[nextItem] = 0;      // Reset count of pixels previously written to this item line
    itemColor[index] = fgColor;        // Remember the fgColor for repaints
    itemSelected[index] = false;       // This item is not yet selected
    itemTxt[index] = new String(txt);  // Remember the text String for repaints

    // Display the text for nextItem line
    DTRACE();
    writeItem((uint8_t *)txt, strlen(txt), fgColor, bgColor);
    DTRACE();
    return index;  // Return index of item just written

}  // setItem()

/**
 * @brief Repaint a specific item in this AListBox
 * @param index Specifies the item
 * @return Index of repainted item or -1 if error
 */
int AListBox::repaint(int index) {
    DTRACE();
    if ((index < 0) || (index >= maxItems)) return -1;
    const char *txt = itemTxt[index]->c_str();
    writeItem((uint8_t *)txt, itemTxt[index]->length(), itemColor[index], bgColor);
    return index;
}  // repaint()

/**
 * @brief Appends an item to the bottom of the list box
 * @param txt Item's NUL-terminated char[] string
 * @param fgColor Foreground text color
 * @return Index of where item was added or -1 if error
 */
int AListBox::addItem(const char *txt, AColor fgColor) {
    // Sanity check
    if (txt == NULL) return -1;
    unsigned index = addItem(nextItem, txt, fgColor);  // Add the item in next location
    return index;
}  // addItem()

/**
 * @brief Add an item
 * @param txt Item's NUL-terminated char[] string
 * @return Index of where item was added or -1 if error
 *
 * The string should not contain a NL char.
 */
int AListBox::addItem(const char *txt) {
    // DTRACE();
    // Sanity check
    if (txt == NULL) return -1;
    unsigned index = addItem(nextItem, txt, fgColor);
    // DTRACE();
    return index;
}

/**
 * @brief Sets an item's color and repaints that item
 * @param index Specifies the item number
 * @param fgColor New foreground color
 * @return Index of modified item or -1 if error
 */
int AListBox::setItemColor(int index, AColor fgColor) {
    // Sanity checks
    if ((index < 0) || (index >= maxItems)) return -1;
    if (itemTxt[index] == NULL) return -1;
    itemColor[index] = fgColor;
    repaint(index);
    return index;
}

/**
 * @brief Write a NL-free buffer to an instance of AListBox
 * @param buffer Data to be written
 * @param size  Number bytes to write
 * @return Number bytes written
 *
 * Places the written text pixels in the line indexed by nextItem.  Does not incremnet
 * nextItem.  Sets up the clip rectangle for the list box.  Writes text with this
 * AListBox's font and colors.  Updates itemLen[] to reflect the long[er] line.
 *
 * Subsequent calls to writeItem() will append to the same item line unless nextItem is
 * modified.
 *
 */
size_t AListBox::writeItem(const uint8_t *bfr, size_t count, AColor fgColor, AColor bgColor) {
    // DPRINTF("buffer='%s', count=%d, fgColor=%04x, bgColor=%04x\n", buffer, count, fgColor, bgColor);

    if (count == 0) return 0;  // Perhaps there's nothing to do

    // Setup the clip rectangle to inside the boundary.
    int16_t clipX = boundary.x1 + 1;
    int16_t clipY = boundary.y1 + 1;
    int16_t clipW = boundary.x2 - boundary.x1 - 1;  // Clip width sans border
    int16_t clipH = boundary.y2 - boundary.y1 - 1;  // Clip height sans border

    // Now adjust the clip to allow room for the border if we have one
    if (hasBorder()) {  // Does list box have a border?
                        // clipX+=1;        // Reserve space for border and one blank pixel
                        // clipY+=1;        // Reserve space for border and one blank pixel
        clipW -= 2;     // Width descreases to make room for border
        clipH -= 2;     // As does height to make room for border
    }
    // TODO:  Someday deal with ridiculous clip rectangle specifications.  For now,
    // garbage in, garbage out.

    // Setup the display clip rectangle for our list box (which better fit on the screen)
    // DPRINTF("clipX=%d,clipY=%d,clipW=%d,clipH=%d, leading=%d\n", clipX, clipY, clipW, clipH, leading);
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Place the cursor where we wish to draw this char[] string
    int16_t drawX = clipX + 1 + itemPixelCount[nextItem];  // Place after existing text on this item line
    int16_t drawY = clipY + 1 + nextItem * leading;        // Place on this item's line
    AGUI::setCursor(drawX, drawY);

    // Draw the text item
    AGUI::setFont(defaultFont);                                   // Config the AListBox font
    AGUI::setTextColor(fgColor, bgColor);                         // Foreground and background text colors
    AGUI::setTextWrap(false);                                     // Clip to screen if clip rectange extends offscreen
    size_t actualCount = AGUI::writeText((uint8_t *)bfr, count);  // Write string to display

    // Recalculate the count of text pixels on this item (so we can later append text to this item)
    ACoord x, y;  // Returned by getTextBounds() but not used
    ACoord w, h;  // Width and height in pixels returned by getTextBounds()
    AGUI::getTextBounds(bfr, (uint16_t)actualCount, drawX, drawY, &x, &y, &w, &h);
    itemPixelCount[nextItem] += (w);  // Accumulated number of horizontal pixels in this item's line of text

    AGUI::setClipRect();
    return actualCount;

}  // writeItem()

/**
 * @brief Determine index of item touched at the given screen coordinates
 * @param xClick Screen x-Coord
 * @param yClick Screen y-Coord
 * @return index of selected item or -1 if none
 */
int AListBox::getSelectedItem(ACoord xClick, ACoord yClick) {
    // Perhaps this click lies entirely outside this AListBox boundary
    if (!boundary.isWithin(xClick, yClick)) return -1;

    // Calculate index of clicked item
    unsigned index = (yClick - boundary.y1) / leading;
    if (index >= maxItems) return -1;  // Validate calculated index

    // Return this index if this item exists else -1
    if (itemPixelCount[index] > 0) return index;
    return -1;  // No such item in this list
}  // getSelection()

void AListBox::deselectItem(int index) {
    if ((index < 0) || (index >= maxItems)) return;  // Garbage?
    DPRINTF("deselectItem(%d)\n", index);
    itemSelected[index] = false;
}

/**
 * @brief This is our private callback override of AWidget invoked when the user clicks in this AListBox
 * @param xClick screen x-coord
 * @param yClick screen y-coord
 *
 * AWidget processTouch() notifies this method when the user clicks in
 * this AListBox.  Our job is to determine which, if any, item was selected
 * and process the selection for that item.
 */
void AListBox::touchWidget(ACoord xClick, ACoord yClick) {
    DTRACE();

    // Find the selected item
    int item = getSelectedItem(xClick, yClick);    // Which item was clicked?
    if ((item < 0) || (item >= maxItems)) return;  // None, nothing to do for this coordinate

    // Toggle status of selected item
    itemSelected[item] = !itemSelected[item];

    // Notify the user-supplied callback of the selected item
    touchItem(item, itemSelected[item]);
}

int AListBox::removeItem(int index) {
    DPRINTF("removeItem(%d) getCount()=%d\n", index,getCount());
    // Sanity check
    if ((index < 0) || (index >= getCount())) return -1;
    if (itemTxt[index] == nullptr) return -1;
    DTRACE();

    // Setup the clip rectangle to inside the boundary.
    int16_t clipX = boundary.x1 + 1;
    int16_t clipY = boundary.y1 + 1;
    int16_t clipW = boundary.x2 - boundary.x1 - 1;  // Clip width sans border
    int16_t clipH = boundary.y2 - boundary.y1 - 1;  // Clip height sans border

    // Now adjust the clip to allow room for the border if we have one
    if (hasBorder()) {  // Does list box have a border?
                        // clipX+=1;        // Reserve space for border and one blank pixel
                        // clipY+=1;        // Reserve space for border and one blank pixel
        clipW -= 2;     // Width descreases to make room for border
        clipH -= 2;     // As does height to make room for border
    }

    // Setup the display clip rectangle for our list box (which better fit on the screen)
    // DPRINTF("clipX=%d,clipY=%d,clipW=%d,clipH=%d, leading=%d\n", clipX, clipY, clipW, clipH, leading);
    AGUI::setClipRect(clipX, clipY, clipW, clipH);

    // Place the cursor where we wish to erase this char[] string
    int16_t drawX = clipX + 1;                    // Place after existing text on this item line
    int16_t drawY = clipY + 1 + index * leading;  // Place on this item's line

    // Recalculate the count of text pixels on this item (so we can later append text to this item)
    ACoord x, y;  // Returned by getTextBounds() but not used
    ACoord w, h;  // Width and height in pixels returned by getTextBounds()
    AGUI::getTextBounds((const uint8_t *)itemTxt[index]->c_str(), itemTxt[index]->length(), drawX, drawY, &x, &y, &w, &h);
    DTRACE();
    // Erase display
    AGUI::fillRect(x, y, w, h+2, bgColor);      //getTextBounds returns undersized h for letter Q
    DTRACE();
    // Restore default clip
    AGUI::setClipRect();

    // Delete the item's data and nullify its dangling pointer
    delete itemTxt[index];
    itemTxt[index] = NULL;
    itemColor[index] = bgColor;
    itemPixelCount[index] = 0;
    itemSelected[index] = false;

    return index;
}

void AListBox::reset(void) {
    DTRACE();
    for (int i = 0; i < maxItems; i++) {
        removeItem(i);
    }
    nextItem = 0;
}

/**
 * @brief Get count of items in list
 * @return count
 */
int AListBox::getCount() {
    return nextItem;  // Index of where next item will be placed
}