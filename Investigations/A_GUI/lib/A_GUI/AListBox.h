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

#include "AGUI.h"
#include "ARect.h"
#include "AWidget.h"

class AListBox : public Print, public AWidget {
   public:
    const static uint8_t maxItems = 24;  // Maximum number of items in a list

    // Constructors/destructors
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h);  // No border
    AListBox(ACoord x1, ACoord y1, ACoord w, ACoord h, AColor borderColor);
    AListBox(ARect boundary, AColor borderColor);
    AListBox(ARect boundary);
    ~AListBox() {}

    // Public methods unique to AListBox
    int drawItem(const char *str);
    int addItem(int index, const char *str, AColor color);
    int addItem(const char *str, AColor color);
    int addItem(const char *str);
    int setItem(int index, const char *str, AColor fgColor, AColor bgColor);
    int getSelectedItem(ACoord xScreen, ACoord yScreen);  // Returns index of item at xClick,yClick
    void deselect(int item);

    void clear(void);
    void clear(int index);
    int getCount(void);

    // Override the Arduino Print interface's virtual methods
    size_t writeItem(const uint8_t *buffer, size_t count);                                  // Writes a char[] string sans NewLine (NL) chars
    size_t writeItem(const uint8_t *buffer, size_t count, AColor fgColor, AColor bgColor);  // Writes a char[] string sans NewLine (NL) chars
    size_t write(uint8_t c) override;                                                       // Write a single char to AListBox
    size_t write(const uint8_t *buffer, size_t size) override;                              // Write a char[] string possibly containing NL chars

   protected:
    //virtual void repaint(void) {}               // Application overrides to receive notifications of repaint events
    virtual void touchItem(int item, bool selected) {}  // Application overrides touchItem() to receive notifications of touch events for items

   private:
    uint16_t leading;            // Space (pixels) between lines of text for this font
    uint16_t itemLen[maxItems];  // Identifies #pixels in each item (or 0 for empty item)
    bool isSelected[maxItems];   // True if indexed item is selected
    uint8_t nextItem;            // Index of where to place next unnumbered addition

    // Helper methods
    void touchWidget(ACoord xScreen, ACoord yScreen) override final;  // We override AWidget touchWidget() to receive touch notifications

};  // AListBox
