#pragma once

#include <Arduino.h>

#include "AGUI.h"
#include "AScrollBox.h"
#include "AWidget.h"

typedef int AScrollBoxToken;

/**
 * @brief Declare the content of a single scroll box item
 */
class AScrollBoxItem {
   public:
    AScrollBoxItem(String str, AColor fgColor, AColor bgColor);
    String str;      // This item's text string
    AColor fgColor;  // This item's foreground color
    AColor bgColor;  // This item's background color
    // AScrollBoxToken token;  // This item's unique, persistent token
    bool selected = false;  // Is this item selected?

   private:
    // AScrollBoxToken generateNewToken();  // Generate a new token value
    // static AScrollBoxToken seedToken;    // The token value to be assigned to the next item added to this AScrollBox
};

/**
 * @brief Declare the content of AScrollBox
 */
class AScrollBox : public AWidget {
   public:
    // Public methods
    AScrollBox(ACoord xCoord, ACoord yCoord, ALength w, ALength h);                        // Build AScrollBox with default colors
    AScrollBoxItem* addItem(String str);                                                   // Add item to bottom of this AScrollBox
    AScrollBoxItem* setItemColors(AScrollBoxItem* token, AColor fgColor, AColor bgColor);  // Change specified item's colors
    int getCount(void);                                                                    // Get count of displayed items in this AScrollBox
    void reset(void);                                                                      // Remove all items from this AScrollBox
    void repaint();                                                                        // Repaint all items in this AScrollBox

    // Public member variables
    const static int maxItems = 24;  // Maximum number of items allowed in AScrollBox

   protected:
    int repaint(int index);                         // Repaint item with specified index (not token) in this AScrollBox
    AScrollBoxItem* repaint(AScrollBoxItem* item);  // Repaint item specified by pointer

   private:
    // Our private methods
    void touchWidget(ACoord screenX, ACoord screenY) override;  // Overrides AWidget to receive touch events for this AScrollBox
    void repaintWidget(void) override;                          // Overrides AWidget to receive repaint events for this AScrollBox
    void scrollUpOneLine();                                     // Scroll all existing items up (by one entry)
    AScrollBoxItem* findItem(AScrollBoxToken token);            // Returns the items[] index of the specified token value
    int getItemIndex(AScrollBoxItem* pItem);                    // Get items[] index of specified item
    bool itemWillFit(int nItems);                               // Helper determines if count items will fit within boundary box

    // Our private member variables
    AScrollBoxItem* displayedItems[maxItems];  // Pointer to items displayed in this AScrollBox indexed by their position
    unsigned leading;                          // The leading (text line spacing in pixels) for this AScrollBox's font
    int nDisplayedItems;                       // Number of *displayed* items
    uint8_t xOffset;                           // #pixels between boundary and text
};
