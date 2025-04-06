#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "AGraphicsDriver.h"
// #include "AColor.h"
#include "ARect.h"

/**
 * @brief This is the base class for all A_GUI widgets.
 *
 */
class AWidget : virtual public AGraphicsDriver {
   public:
    AWidget(void);
    ~AWidget(void);
    static void processTouch(uint16_t xCoord, uint16_t yCoord);  // All touch events pass through processTouch()

   protected:
    static AWidget* allWidgets;                                    // Head of the unordered list of all widgets
    AWidget* next;                                                 // Next widget in the unordered list of all widgets
    ARect boundary;                                                // Widget boundary rectangle coordinates
    AColor bdColor;                                                // Border color
    AColor bgColor;                                                // Background color
    AColor fgColor;                                                // Foreground color
    virtual void doTouchWidget(ACoord screenX, ACoord screenY) {}  // Derived class must overide doTouchWidget() to process its touch event
    virtual void doRepaintWidget(void) {}                          // Derived class must overide doRepaintWidget() to repaint itself
};
