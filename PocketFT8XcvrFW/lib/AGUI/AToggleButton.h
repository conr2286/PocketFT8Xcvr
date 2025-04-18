/**
 * AToggleButton --- An Arduino-friendly GUI Button
 *
 * @author Jim Conrad, KQ7B
 *
 * @license MIT License
 * @copyright 2025 Jim Conrad, All rights reserved.
 *
 * AToggleButton implements a very simple button
 * AListBox is a AWidget
 *
 * AToggleButton uses a callBack function for clicks.
 *
 */

#pragma once

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"

class AToggleButton : public AWidget {
   public:
    // Constructors
    AToggleButton(const char* str, ACoord x1, ACoord y1, ALength w, ALength h, bool border=true);
    virtual ~AToggleButton() {}

    // Public methods
    void enable(void);                  // Enable this button
    void disable(void);                 // Disable (grey) this button
    void repaintWidget(void) override;  // Repaint this button

    // Protected
    virtual void touchButton(void) {}  // Application overrides to receive notifications of touch events for this AToggleButton

   private:
    void touchWidget(ACoord xScreen, ACoord yScreen) override final;  // We override AWidget touchWidget() to receive touch notifications

    // Private variables
    bool enabled;  // Enabled or disabled (greyed)
    bool state;    // Button state (on or off)
    String str;    // The button's text string
};  // AToggleButton
