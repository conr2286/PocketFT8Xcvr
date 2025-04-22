#pragma once

#include <Arduino.h>

#include "AGUI.h"
#include "AListBox.h"
#include "AWidget.h"

class AListBox;

/**
 * @brief Declare the content of a single scroll box item
 */
class AListBoxItem final {
   public:
    AListBoxItem(String str, AColor fgColor, AColor bgColor, AListBox* pBox);
    String str;                                          // This item's text string
    AColor fgColor;                                      // This item's foreground color
    AColor bgColor;                                      // This item's background color
    bool selected = false;                               // Is this item selected?
    void repaint(void);                                  // Repaint this item
    void setItemColors(AColor fg, AColor bg = A_BLACK);  // Change colors
    void setItemText(String str, AColor fg = A_WHITE);   // Change specified item's text string
    int getIndex(void);                                  // Get an item's index

   protected:
    AListBox* listBoxContainer;  // Back pointer to containing AListBox
};

/**
 * @brief Declare the content of AListBox
 */
class AListBox : public AWidget {
   public:
    // Public methods
    AListBox(ACoord xCoord, ACoord yCoord, ALength w, ALength h, AColor bdColor = A_BLACK);  // Build AListBox with default colors
    virtual ~AListBox();                                                                     // Destructor purges data struct and erases displayed items
    AListBoxItem* addItem(AListBox* pListBox, String str, AColor fg = A_WHITE);              // Add item to bottom of this AListBox
    int setItem(int index, String str, AColor fg, AColor bg = A_BLACK);                      // Add item at specified index
    AListBoxItem* setItemColors(AListBoxItem* pItem, AColor fg, AColor bg = A_BLACK);        // Change specified item's colors
    int setItemColor(int index, AColor fg, AColor bg = A_BLACK);                             // Change indexed item's foreground color
    int getCount(void);                                                                      // Get count of displayed items in this AListBox
    void reset(void);                                                                        // Remove all items from this AListBox
    int getItemIndex(AListBoxItem* pItem);                                                   // Get items[] index (line number) of specified item
    AListBoxItem* repaint(AListBoxItem* pItem);                                              // Repaint item specified by pointer

    // Public constants
    constexpr static int maxItems = 16;  // Maximum number of items allowed in AListBox

   protected:
    virtual void touchItem(AListBoxItem* pItem) {}  // Application overrides touchItem() to receive notifications of touch events

   private:
    // Our private methods
    void touchWidget(ACoord screenX, ACoord screenY) override;      // We override AWidget to receive touch events for this AListBox
    AListBoxItem* getSelectedItem(ACoord screenX, ACoord screenY);  // Return pointer to selected item
    void repaintWidget(void) override;                              // We override AWidget to receive repaint events for this AListBox
    // bool itemWillFit(int nItems);                                     // Helper determines if count items will fit within boundary box
    int removeItem(int index);  // Remove specified item from displayedItems[]
    int repaint(int index);     // Repaint item specified by index

    // Our private member variables
    AListBoxItem* displayedItems[maxItems];  // Pointer to items displayed in this AListBox indexed by their position
    unsigned leading;                        // The leading (text line spacing in pixels) for this AListBox's font
    int nDisplayedItems;                     // Number of *displayed* items
    constexpr static uint8_t xOffset = 3;    // xOffset==n provides n-1 blank pixels on left, between border and text
    constexpr static uint8_t yOffset = 2;    // Similar but for space at top of box
};
