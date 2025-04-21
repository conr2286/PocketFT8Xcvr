/**
 * AListBox --- An Arduino-friendly GUI List Box
 *
 * @author Jim Conrad, KQ7B
 *
 * @license MIT License
 * @copyright 2025 Jim Conrad, All rights reserved.
 *
 * AListBox implements the Arduino Print interface
 * AListBox is a AWidget
 *
 * Both AWidget and AListBox have callBack function pointers for selections.  The AWidget
 * callBack deals with clicks on the AListBox while the AListBox callBack member deals
 * with items in the list box.
 *
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>

//#include <vector>

#include "AGUI.h"
#include "ARect.h"
#include "AWidget.h"

class AListBox : public AWidget {
   public:
    const static uint8_t maxItems = 24;  // Maximum number of items in a list

    // Constructors/destructors
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h);  // No border
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h, AColor borderColor);
    virtual ~AListBox() {}

    // Public methods unique to AListBox
    int addItem(const char *txt, AColor fgColor);                             // Add a new item with specified foreground color
    int addItem(const char *txt);                                             // Add a new item
    int setItem(int index, const char *str, AColor fgColor, AColor bgColor);  // Assign values to this item
    int setItemColor(int index, AColor fgColor);                              // Change an item's foreground color
    int getSelectedItem(ACoord xScreen, ACoord yScreen);                      // Returns index of item at xClick,yClick
    void deselectItem(int item);                                              // Deselect specified item
    int removeItem(int index);                                                // Remove the specified item
    void reset(void);                                                         // Reset (clear all items) this AListBox

    int getCount(void);  // #items

    void repaint(void);  // Repaint every item

    // Override the Arduino Print interface's virtual methods
    size_t writeItem(const uint8_t *buffer, size_t count, AColor fgColor, AColor bgColor);  // Writes a char[] string sans NewLine (NL) chars

   protected:
    virtual void touchItem(int item, bool selected) {}  // Application overrides touchItem() to receive notifications of touch events for items
    int repaint(int index);                             // Repaint a specific item

   private:
    int addItem(int index, const char *txt, AColor fgColor);  // Add iteam at index with specified foreground color
    uint16_t leading;                                         // Space (pixels) between lines of text for this font
    uint16_t itemPixelCount[maxItems];                        // Identifies #pixels in each item (or 0 for empty item)
    bool itemSelected[maxItems];                              // True if indexed item is selected
    uint8_t nextItem;                                         // Index of where to place next unnumbered addition
    String *itemTxt[maxItems];                                // The item text
    AColor itemColor[maxItems];                               // the items' fgColors

    // Helper methods
    void touchWidget(ACoord xScreen, ACoord yScreen) override final;  // We override AWidget touchWidget() to receive touch notifications

};  // AListBox
