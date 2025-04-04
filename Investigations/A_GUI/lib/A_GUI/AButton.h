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

#include "AGraphicsDriver.h"
#include "AWidget.h"

class AButton : public AWidget {
   public:
    // Constructors
    AButton(const char *str, ACoord x1, ACoord y1, ACoord w, ACoord h);

    // Public methods
    int setCallback(void *(*callback)(bool stateOn));  // Set user's callback function
    void enable(void);                                 // Enable this button
    void disable(void);                                // Disable (grey) this button
    void setState(bool onOff);                         // Set button state on or off
    void setText(const char *str);                     // Set button text

   private:
    // Private variables
    bool enabled;  // Enabled or disabled (greyed)
    bool stateOn;  // Button state (on or off)
};  // AListBox