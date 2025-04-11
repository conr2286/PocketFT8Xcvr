#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "AGUI.h"
#include "ARect.h"

/**
 * @brief This is the base class for all A_GUI widgets.
 *
 */
class AWidget {
   public:
    AWidget(void);
    virtual ~AWidget(void);
    static void processTouch(uint16_t xCoord, uint16_t yCoord);  // All touch events pass through processTouch()
    bool hasBorder(void);
    inline ALength width(void) { return boundary.w; }
    inline ALength height(void) { return boundary.h; }

   protected:
    static AWidget* allWidgets;  // Head of the unordered list of all widgets
    AWidget* next;               // Next widget in the unordered list of all widgets
    ARect boundary;              // Widget boundary rectangle coordinates
    ACoord radius;               // Rounded corner radius
    AColor bdColor;              // Border color
    AColor bgColor;              // Background color
    AColor fgColor;              // Foreground color
    AColor spColor;              // Special color (e.g. selected item color)
    const GFXfont* defaultFont;  // Default font

    virtual void touchWidget(ACoord screenX, ACoord screenY) {}  // Derived classes must overide touchWidget() to receive touch events
    virtual void repaintWidget(void) {}                            // Derived classes must overide doRepaintWidget() to repaint themselves
};
