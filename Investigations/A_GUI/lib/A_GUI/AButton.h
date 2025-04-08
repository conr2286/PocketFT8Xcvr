/**
 * AButton --- An Arduino-friendly GUI Button
 *
 * @author Jim Conrad, KQ7B
 *
 * @license MIT License
 * @copyright 2025 Jim Conrad, All rights reserved.
 *
 * AButton implements a very simple button
 * AListBox is a AWidget
 *
 * AButton uses a callBack function for clicks.
 *
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "AGUI.h"
#include "AWidget.h"

class AButton : public AWidget {
   public:
    // Constructors
    AButton(const char *str, ACoord x1, ACoord y1, ACoord w, ACoord h);

    // Public methods
    void enable(void);   // Enable this button
    void disable(void);  // Disable (grey) this button

   private:
    // Private variables
    ACoord r;      // Radius of the rounded corners
    bool enabled;  // Enabled or disabled (greyed)
    bool stateOn;  // Button state (on or off)
};  // AListBox