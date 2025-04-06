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

#include "AGraphicsDriver.h"
#include "ARect.h"
#include "AWidget.h"

class AListBox : public AWidget, public Print {
   public:
    const static uint8_t maxItems = 24;  // Maximum number of items in a list

    // Constructors
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h);  // No border
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h, AColor borderColor);
    AListBox(ARect boundary, AColor borderColor);
    AListBox(ARect boundary);

    // Public methods unique to AListBox
    int drawItem(const char *str);
    int addItem(int index, const char *str, AColor color);
    int addItem(const char *str, AColor color);
    int addItem(const char *str);
    int setItem(int index, const char *str, AColor fgColor, AColor bgColor);

    void setSelection(void (*doSelection)(int index));    // Set user-supplied callback function for this AListBox
    int getSelectedItem(ACoord xScreen, ACoord yScreen);  // Returns index of item selected at xClick,yClick

    // int getSelection(unsigned xScreen, unsigned yScreen);

    // void setDefaultColor(AColor color);
    // void setBackgroundColor(AColor color);
    void clear(void);
    void clear(int index);
    int getCount(void);
    // void setBorderColor(AColor color);

    // Override the Arduino Print interface's virtual methods
    size_t writeItem(const uint8_t *buffer, size_t count);      // Writes a char[] string sans NewLine (NL) chars
    size_t write(uint8_t c) override;                           // Write a single char to AListBox
    size_t write(const uint8_t *buffer, size_t size) override;  // Write a char[] string possibly containing NL chars

   protected:
    void doTouchWidget(ACoord xScreen, ACoord yScreen) override;  // AWidget::processTouch() notifies our callback function when this AListBox is touched
    virtual void doRepaintAListBox(void) {}                       // User overrides doRepaintAlistBox() to receive notifications of repaint events
    virtual void doTouchItem(int item) {}                         // User overrides doTouchItem() to receive notifications of touch events for items

   private:
    AColor siColor;              // Selected item color
    uint16_t leading;            // Space (pixels) between lines of text for this font
    uint16_t itemLen[maxItems];  // Identifies #pixels in each item (or 0 for empty item)
    bool isSelected[maxItems];   // True if indexed item is selected
    uint8_t nextItem;            // Index of where to place next unnumbered addition

    // Helper methods
    bool hasBorder();  // Returns true if list box has a border

};  // AListBox