/**
 * @brief ATextBox is a non-interactive widget for displaying text
 *
 */

#pragma once

#include <Arduino.h>

// #include <string>

#include "AGUI.h"
#include "AWidget.h"

class ATextBox : public AWidget {
   public:
    // Constructors/Destructors
    ATextBox(const char* txt, ACoord x1, ACoord y1, ALength w, ALength h, AColor border);
    virtual ~ATextBox() {}

    // Public methods
    void setText(const char* str, AColor fg = A_WHITE);
    void setText(String& str, AColor fg = A_WHITE);
    void onRepaintWidget(void) override;  // Repaint this ATextBox
    void reset(void);                     // Clear box of all text

   protected:
   private:
    void onTouchWidget(ACoord xScreen, ACoord yScreen) override {};  // We implement and ignore AWidget's touch notifications

    // Private variables
    ACoord r;    // Radius of the box's rounded corners
    String str;  // Copy of the text string object
};  // AToggleButton
