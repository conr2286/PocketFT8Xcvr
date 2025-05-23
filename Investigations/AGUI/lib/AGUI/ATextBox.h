/**
 * @brief ATextBox is a non-interactive widget for displaying text
 *
 */

#pragma once

#include <Arduino.h>

#include <string>

#include "AGUI.h"
#include "AWidget.h"

class ATextBox : public AWidget {
   public:
    // Constructors/Destructors
    ATextBox(const char* txt, ACoord x1, ACoord y1, ALength w, ALength h, bool border=true);

    // Public methods
    void setText(const char* str);
    void repaintWidget(void) override;  // Repaint this ATextBox


    // Public variables
    ARect boundary;

   protected:

   private:
    void touchWidget(ACoord xScreen, ACoord yScreen) {};  // We implement and ignore AWidget's touch notifications

    // Private variables
    ACoord r;    // Radius of the box's rounded corners
    String str;  // The text string object
};  // AToggleButton
