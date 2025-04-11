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
 *  + AListBox does not store the item data
 *  + Repaints handled by the application
 *  + AListBox does not support scrolling
 *  + AListBox does not support multiple selections
 *  + AListBox cannot be resized
 *  + Selections are polled (see getSelection() description)
 *  + The text font uses its widget's default
 *
 * USAGE
 *  + An application should create a derived class from AListBox overriding its virtual methods.
 *  + The derived class should implement the touchItem() method to handle touch events for its items.
 *  + The derived class may implement the doRepaintAListBox() method to handle repaintWidget events.
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
// #include <Fonts/FreeSerif9pt7b.h>
#include <SPI.h>
#include <stdint.h>
#include <string.h>

#include "AGUI.h"
#include "AWidget.h"
#include "DEBUG.h"
//#include "FT8Font.h"  //Include the default font

// using namespace agui;

/**
 * @brief Build AListBox sans border line using width and height
 * @param x1 Upper-left corner pixel x-coord
 * @param y1 Upper-left corner pixel y-coord
 * @param w Width pixels
 * @param h Height pixels
 */
AListBox::AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h) {
    if (!Serial) Serial.begin(9600);
    // DPRINTF("AListBox()=%08x\n", this);
    // DPRINTF("x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);

    // Remember location and extent of the boundary box
    boundary.setCorners(x1, y1, w, h);
    // DTRACE();

    // Initialize some default text values for this new list box
    // DPRINTF("defaultFont=0x%x\n", defaultFont);
    // AGUI::setFont(defaultFont);
    // DTRACE();
    leading = AGUI::getLeading();  // Get the leading (in pixels) for this font
    // DTRACE();

    // Decorate the list box
    AGUI::setClipRect();  // Clear any existing clip
    // DTRACE();
    AGUI::fillRoundRect(x1, y1, w, h, radius, bgColor);  // Background
    // DTRACE();

    // No items exist yet and none are selected
    for (int i = 0; i < maxItems; i++) {
        itemPixelCount[i] = 0;
        isSelected[i] = false;
    }

    // The first item will be item 0
    nextItem = 0;  // Index of where addItem() places unnumbered additions
    // DTRACE();
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
    // DPRINTF("Border:  x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);
    AGUI::drawRoundRect(x1, y1, w, h, radius, bdColor);  // Draw the border (might be just background)

    // DTRACE();
}  // AListBox()

/**
 * @brief Build AListBox using a bounding-rectangle with a border
 * @param boundary The bounding-rectangle
 * @param borderColor The border line color
 */
AListBox::AListBox(ARect boundary, AColor borderColor) : AListBox(boundary.x1, boundary.y1, boundary.w, boundary.h, borderColor) {
    if (!Serial) Serial.begin(9600);
    // DPRINTF("x1=%d, y1=%d, x2=%d, y2=%d, bdColor=0x%x\n", boundary.x1, boundary.y1, boundary.x2, boundary.y2, bdColor);
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
 * Advances nextItem index to the following line.
 */
int AListBox::addItem(int index, const char *txt, AColor fgColor) {
    // DPRINTF("addItem(%d,%p,%04x) bgColor=%04x", index, str, fgColor, bgColor);

    // Display the item
    int result = setItem(index, txt, fgColor, bgColor);

    // Advance nextItem for subsequent additions
    if (result >= 0) nextItem++;  // This might increment beyond the AListBox clip rectangle
    return result;                // Return index of item just written

}  // addItem()

/**
 * @brief Places an item at index without incrementing nextItem
 * @param index Specifies index of item
 * @param txt Nul-terminated string to place
 * @param fgColor Foreground color of the item's text
 * @param bgColor Background color of the item's text
 * @return
 */
int AListBox::setItem(int index, const char *txt, AColor fgColor, AColor bgColor) {
    // DPRINTF("setItem(%d, %s, %04x, %04x)\n ", index, str, fgColor, bgColor);

    //  Sanity checks
    if (index >= maxItems) return -1;
    if (txt == NULL) return -1;

    // Truncate text containing a NL
    char *pNL = strchr(txt, '\n');
    if (pNL != NULL) *pNL = 0;

    // Overwrite any existing item at index
    nextItem = index;              // Informs writeItem() where to write
    itemPixelCount[nextItem] = 0;  // Reset count of pixels previously written to this item line
    items[index] = String(txt);     // Record the text String for repaints

    // Display the text for nextItem line
    // writeItem((uint8_t *)str, strlen(str));  // Output the text
    writeItem((uint8_t *)txt, strlen(txt), fgColor, bgColor);
    return index;  // Return index of item just written
}

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
 * @brief Write item
 * @param bfr char[] to write
 * @param count #chars to write
 * @return #chars actually written
 */
size_t AListBox::writeItem(const uint8_t *bfr, size_t count) {
    DPRINTF("buffer=%s, fgColor=%04x, bgColor=%04x\n", bfr, fgColor, bgColor);
    return writeItem(bfr, count, fgColor, bgColor);
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

    return actualCount;

}  // write()

/**
 * @brief Write the specified character to an instance of AListBox
 * @param c The character to write
 * @return Number of bytes actually written
 *
 * The character is appended to the existing text, if any, at nextItem.  Successive
 * write(c) will continue on the same line.  Writing a NL advances nextItem to
 * continue on the next line.
 */
size_t AListBox::write(uint8_t c) {
    write(&c, 1);
    return 1;
}  // write()

/**
 * @brief Write bfr to nextItem in the list box
 * @param bfr The unsigned char[] of chars to write
 * @param count #chars to write
 * @return Number chars actually written
 *
 * The NUL terminator in bfr, if any, is ignored.  NL chars will advance nextItem
 * to the next item line.
 */
size_t AListBox::write(const uint8_t *bfr, size_t count) {
    if (!Serial) Serial.begin(9600);

    DPRINTF("bfr='%s', count=%d\n", bfr, count);
    uint8_t *pSegment = (uint8_t *)bfr;  // Points to first char of current segment
    uint8_t *pScan = pSegment;           // Scans a segment in search of NL char
    size_t n = 0;                        // Count used to terminate loop after processing count chars

    // Perhaps there's nothing to do
    if (count == 0) return 0;

    // Loop breaks bfr into segments terminated by a NL-or-NUL.
    while (n < count) {
        DPRINTF("bfr[%d]='%c'\n", n, bfr[n]);
        // Have we found a NL char in this segment?
        if (*pScan == '\n') {
            // Yes, write the current segment to the display
            size_t nSegment = pScan - pSegment;  // Number chars in this segment sans NL
            writeItem(pSegment, nSegment);       // Write chars in this segment to display
            pSegment += nSegment;                // Advance segment pointer past NL
            nextItem++;                          // Advance index to next item
        }
        pScan++;  // Advance scan pointer to next char
        n++;      // And advance the count of chars processed from bfr[]
    }  // while

    // If bfr[] isn't terminated with a NL then we need to write the final segment
    if (bfr[n - 1] != '\n') {
        size_t nLastSegment = pScan - pSegment;  // Number chars in final segment
        writeItem(pSegment, nLastSegment);       // Write chars in final segment to display
        // Note that we don't advance nextItem since bfr[] didn't end with a NL
    }

    return count;

}  // write()

/**
 * @brief Determine which item is clicked at the given screen coordinates
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

void AListBox::deselect(int index) {
    if ((index < 0) || (index >= maxItems)) return;  // Garbage?
    DPRINTF("deselect(%d)\n", index);
    isSelected[index] = false;
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
    isSelected[item] = !isSelected[item];

    // Notify the user-supplied callback of the selected item
    touchItem(item, isSelected[item]);
}
