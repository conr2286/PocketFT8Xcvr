#pragma once

#include <Arduino.h>

#include "AGUI.h"
#include "AScrollBox.h"
#include "AWidget.h"

typedef int AScrollBoxToken;

class AScrollBox;

/**
 * @brief Declare the content of a single scroll box item
 */
class AScrollBoxItem {
   public:
    AScrollBoxItem(String& str, AColor fgColor, AColor bgColor, AScrollBox* box);
    virtual ~AScrollBoxItem() {}
    String str;                                         // This item's text string copy
    AColor fgColor;                                     // This item's foreground color
    AColor bgColor;                                     // This item's background color
    bool selected = false;                              // Is this item selected?
    void repaint(void);                                 // Repaint this item
    void setItemColors(AColor fg, AColor bg);           // Change colors
    void setItemText(String str, AColor fg = A_WHITE);  // Change specified item's text String
    String* getItemText(void);                          // Get reference to this item's text String
    void reset(void);                                   // Clear the box of text

    unsigned long timeStamp;  // Time stamp for removing ancient items

   private:
    AScrollBox* scrollBoxContainer;
};

/**
 * @brief Declare the content of AScrollBox
 */
class AScrollBox : public AWidget {
   public:
    // Public methods
    AScrollBox(ACoord xCoord, ACoord yCoord, ALength w, ALength h, AColor bdColor);     // Build AScrollBox with default colors
    virtual ~AScrollBox();                                                              // Destructor purges data struct and erases displayed items
    AScrollBoxItem* addItem(AScrollBox* pAScrollBox, String str, AColor fg = A_WHITE);  // Add item to bottom of this AScrollBox
    AScrollBoxItem* setItemColors(AScrollBoxItem* pItem, AColor fg, AColor bg);         // Change specified item's colors
    int getCount(void) const;                                                           // Get count of displayed items in this AScrollBox
    void reset(void);                                                                   // Remove all items from this AScrollBox
    int getItemIndex(AScrollBoxItem* pItem) const;                                      // Get items[] index (line number) of specified item
    AScrollBoxItem* repaint(AScrollBoxItem* pItem);                                     // Repaint item specified by pointer
    void reviewTimeStamps(void);                                                        // Review item timestamps and scroll if ancient

    // Public constants
    constexpr static int maxItems = 16;  // Maximum number of items allowed in AScrollBox

   protected:
    virtual void onTouchItem(AScrollBoxItem* pItem) {}  // Application overrides onTouchItem() to receive notifications of touch events
    void scrollUpOneLine();                             // Scroll all existing items up (by one entry)
    bool itemWillFit(int nItems) const;                 // Helper determines if count items will fit within boundary box

    // Protected variablez
    int nDisplayedItems;                       // Number of *displayed* items
    AScrollBoxItem* displayedItems[maxItems];  // Pointer to items displayed in this AScrollBox indexed by their position

   private:
    // Our private methods
    void onTouchWidget(ACoord screenX, ACoord screenY) override;            // We override AWidget to receive touch events for this AScrollBox
    AScrollBoxItem* getSelectedItem(ACoord screenX, ACoord screenY) const;  // Return pointer to selected item
    void onRepaintWidget(void) override;                                    // We override AWidget to receive repaint events for this AScrollBox
    int removeItem(int index);                                              // Remove specified item from displayedItems[]
    int repaint(int index);                                                 // Repaint item specified by index

    // Our private member variables
    unsigned leading;                      // The leading (text line spacing in pixels) for this AScrollBox's font
    constexpr static uint8_t xOffset = 3;  // xOffset==n provides n-1 blank pixels on left, between border and text
    constexpr static uint8_t yOffset = 2;  // Similar but for space at top of box
};
