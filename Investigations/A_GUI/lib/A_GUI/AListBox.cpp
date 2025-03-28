/**
 * AListBox implements a very simplified GUI ListBox widget
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
 *  + Selections
 *
 * SIMPLIFICATIONS
 *  + AListBox does not store the item data
 *  + No repaint (it must be manually re-populated)
 *  + AListBox cannot be resized
 *  + Selections are polled (see getSelection() description)
 *  + For now, the text font is cast-in-brass as fonts are expensive and awkward in GFX
 *
 * MISC
 * AListBox "is a" AWidget, and it implements the Print interface.
 *
 * Items in AListBox are drawn with the display's current font, size and color.
 *
 * While AListBox cannot repaint itself, the user can supply an optional getItemText() callback
 * invoked when the user experience can be improved by redrawing the text.  For example, AListBox
 * invokes this function, if available, to highlight a selected item's text.
 *
 * Items are not widgets or even objects.
 *
 */

#include "AListBox.h"

#include <Adafruit_GFX.h>  //Note:  GFX must include prior to HX8357
#include <Arduino.h>
#include <Fonts/FreeMono9pt7b.h>
#include <stdint.h>
#include <string.h>

#include "ACoord.h"
#include "AWidget.h"
#include "DEBUG.h"
#include "HX8357_t3n.h"

/**
 * @brief Build AListBox sans border line using width and height
 * @param tft Display
 * @param x1 Upper-left corner pixel x-coord
 * @param y1 Upper-left corner pixel y-coord
 * @param w Width pixels
 * @param h Height pixels
 */
AListBox::AListBox(HX8357_t3n *tft, ACoord x1, ACoord y1, ACoord w, ACoord h) {
    if (!Serial) Serial.begin(9600);
    DPRINTF("x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);

    // Bind to the display
    this->tft = tft;

    // Remember location and extent of the boundary box
    boundary.setCorners(x1, y1, x1 + w, y1 + h);

    // Initialize some default values for the list box
    fgColor = WHITE;              // Text color
    bgColor = BLACK;              // Text background
    bdColor = BLACK;              // bdColor==bgColor ==> no border
    siColor = GREY;               // Selected text background color
    txtFont = &FreeMono9pt7b;     // For now, the font is cast-in-brass
    tft->setFont(txtFont);        // Setup display for this font
    leading = tft->getLeading();  // Get the leading (in pixels) for this font
    getItemText = NULL;           // Assume user can't resupply text strings for this box

    // Decorate the list box
    tft->setClipRect();                    // Clear any existing clip
    tft->fillRect(x1, y1, w, h, bgColor);  // Background

    // All items are currently empty
    for (int i = 0; i < maxItems; i++) itemLen[i] = 0;
    nextItem = 0;  // Index of where addItem() places first unnumbered addition
}  // AListBox()

/**
 * @brief Build AListBox with a border line using width and height
 * @param tft Display
 * @param x1 Upper-left corner pixel x-coord
 * @param y1 Upper-left corner pixel y-coord
 * @param w Width pixels
 * @param h Height pixels
 * @param bdColor The border line's color
 */
AListBox::AListBox(HX8357_t3n *tft, ACoord x1, ACoord y1, ACoord w, ACoord h, AColor bdColor) : AListBox(tft, x1, y1, w, h) {
    if (!Serial) Serial.begin(9600);
    DPRINTF("x1=%d, y1=%d, w=%d, h=%d, bdColor=0x%x\n", x1, y1, w, h, bdColor);

    // Initialize additional default values for this list box
    this->bdColor = bdColor;  // Border color

    // Decorate the list box
    tft->drawRect(x1, y1, w, h, bdColor);  // Draw the border (might be just background)

    DTRACE();
}  // AListBox()

/**
 * @brief Build AListBox with a border using a bounding-rectangle
 * @param tft The display
 * @param boundary The bounding-rectangle
 * @param borderColor The border line color
 */
AListBox::AListBox(HX8357_t3n *tft, ARect boundary, AColor borderColor) : AListBox(tft, boundary.x1, boundary.y1, boundary.x2 - boundary.x1, boundary.y2 - boundary.y1, borderColor) {
    DPRINTF("x1=%d, y1=%d, x2=%d, y2=%d, bdColor=0x%x\n", boundary.x1, boundary.y1, boundary.x2, boundary.y2, bdColor);
}  // AListBox()

/**
 * @brief Add an item to the list box at the specified index
 * @param index Specifies which item in the list
 * @param str NUL-terminated char[] string text to draw at the list item
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
int AListBox::addItem(int index, const char *str, AColor fgColor) {
    DTRACE();
    // Sanity checks
    if (index >= maxItems) return -1;

    // Truncate text containing a NL
    char *pNL = strchr(str, '\n');
    if (pNL != NULL) *pNL = 0;

    // Overwrite any existing text at index
    nextItem = index;       // Informs writeItem() where to write
    itemLen[nextItem] = 0;  // Reset count of pixels previously written to this item line

    // Display the text for nextItem line
    writeItem((uint8_t *)str, strlen(str));  // Output the text

    // Advance nextItem for subsequent additions
    nextItem++;    // This might increment beyond the AListBox clip rectangle
    DPRINTF("nextItem=%d\n", nextItem);
    return index;  // Return index of item just written

}  // addItem()

/**
 * @brief Sequentially adds an item to bottom of the list box
 * @param str Item's NUL-terminated char[] string
 * @param fgColor Foreground text color
 * @return Index of where item was added or -1 if error
 */
int AListBox::addItem(const char *str, AColor fgColor) {
    unsigned index = addItem(nextItem, str, fgColor);  // Add the item in next location
    return index;
}  // addItem()

/**
 * @brief Add an item
 * @param str Item's NUL-terminated char[] string
 * @return Index of where item was added or -1 if error
 *
 * The string should not contain a NL char.
 */
int AListBox::addItem(const char *str) {
    DTRACE();
    unsigned index = addItem(nextItem, str, fgColor);
    DTRACE();
    return index;
}

/**
 * @brief Determine if a list box has a drawn border
 * @return true if has a border, else false
 *
 * A list box has a border if its border color != its background color
 */
bool AListBox::hasBorder() {
    return bdColor != bgColor;
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
size_t AListBox::writeItem(const uint8_t *buffer, size_t count) {
    DPRINTF("buffer='%s', count=%d\n", buffer, count);

    if (count == 0) return 0;  // Perhaps there's nothing to do

    // Setup the clip rectangle for this AListBox.  We always reserve one blank pixel on
    // every side so adjacent boxes have some minimal clearance.  If we have a border,
    // then we later allow another pixel on each side for drawing the border.
    int16_t clipX = boundary.x1 + 1;                // Always reserve one blank pixel left of text
    int16_t clipY = boundary.y1 + 1;                // Always reserve one blank pixel above text
    int16_t clipW = boundary.x2 - boundary.x1 - 1;  // Clip width sans border
    int16_t clipH = boundary.y2 - boundary.y1 - 1;  // Clip height sans border
    //DPRINTF("clipX=%d,clipY=%d,clipW=%d\n", clipX, clipY, clipW);
    //DPRINTF("boundary.y1=%d, boundary.y2=%d, clipH=%d\n", boundary.y1, boundary.y2, clipH);

    // Now adjust the clip for the border if we have one.
    if (hasBorder()) {  // Does list box have a border?
        clipX++;        // Reserve another pixel on the left
        clipY++;        // Reserve another pixel on the top
        clipW -= 2;     // Width descreases by two
        clipH -= 2;     // As does height
        DPRINTF("clipX=%d,clipY=%d,clipW=%d,clipH=%d\n", clipX, clipY, clipW, clipH);
    }

    // TODO:  Someday deal with ridiculous clip rectangle specifications.  For now, ask
    // for garbage, get something smelly.

    // Setup the display clip rectangle for our list box (which better fit on the screen)
    //DPRINTF("clipX=%d,clipY=%d,clipW=%d,clipH=%d\n", clipX, clipY, clipW, clipH);
    tft->setClipRect(clipX, clipY, clipW, clipH);

    // Place the cursor where we wish to draw this char[] string
    int16_t drawX = clipX + itemLen[nextItem] + 1;   // Place after existing text on this item line
    int16_t drawY = clipY + nextItem * leading + 1;  // Place on this item's line
    DPRINTF("drawX=%d, drawY=%d\n", drawX, drawY);
    tft->setCursor(drawX, drawY);
    //DTRACE();

    // Draw the text item
    tft->setFont(txtFont);  // Config the font we're using in this AListBox
    //DTRACE();
    tft->setTextColor(fgColor, bgColor);  // Foreground and background text colors
    tft->setTextWrap(false);              // Clip to screen if clip rectange extends offscreen
    DPRINTF("buffer='%s', count=%d\n", buffer, count);
    size_t actualCount = tft->write((uint8_t *)buffer, count);  // Write string to display
    //DTRACE();

    // Recalculate the count of text pixels on this item
    int16_t x1, y1;  // Returned by getTextBounds() but not used
    uint16_t w, h;   // Width and height in pixels returned by getTextBounds()
    tft->getTextBounds(buffer, (uint16_t)actualCount, drawX, drawY, &x1, &y1, &w, &h);
    DPRINTF("x1=%d, y1=%d, w=%d, h=%d\n", x1, y1, w, h);
    itemLen[nextItem] += (w);  // Accumulated number of horizontal pixels in this item's line of text

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
    DPRINTF("write(%c)\n", c);
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
    DPRINTF("bfr='%s', count=%d\n", bfr, count);
    uint8_t *pSegment = (uint8_t *)bfr;  // Points to first char of current segment
    uint8_t *pScan = pSegment;           // Scans a segment in search of NL char
    size_t n = 0;                        // Count used to terminate loop after processing count chars

    //Perhaps there's nothing to do
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
        n++;    // And advance the count of chars processed from bfr[]
    }  // while

    //If bfr[] isn't terminated with a NL then we need to write the final segment
    if (bfr[n-1] != '\n') {
        size_t nLastSegment = pScan - pSegment; //Number chars in final segment
        writeItem(pSegment, nLastSegment);      //Write chars in final segment to display
        //Note that we don't advance nextItem since bfr[] didn't end with a NL
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
    // Perhaps this click lies outside this AListBox boundary
    if (!boundary.isWithin(xClick, yClick)) return -1;

    // Calculate index of clicked item
    unsigned index = (yClick - boundary.y1) / leading;

    // Return this index if this item exists else -1
    if (itemLen[index] > 0) return index;
    return -1;  // No such item in this list
}  // getSelection()

/**
 * @brief Performs an item selection for the specified screen coordinates
 * @param xClick screen x-coord
 * @param yClick screen y-coord
 *
 * The Widget's doSelections() notifies this method when the user clicks in
 * this AListBox.  Our job is to determine which, if any, item was selected
 * and process the selection for that item.
 */
void AListBox::selection(ACoord xClick, ACoord yClick) {
    char *selectedText = NULL;
    // Find the selected item
    int index = getSelectedItem(xClick, yClick);  // Which item was clicked?
    if (index < 0) return;                        // None, nothing to do for this coordinate

    // Highlight the selected item using siColor only if the user can resupply the item's text
    if (getItemText != NULL) selectedText = getItemText(index);
    addItem(index, selectedText, siColor);  // TODO:  this screws up nextItem.  Need setItem()???

    // Notify the user-supplied callback about selected item
}