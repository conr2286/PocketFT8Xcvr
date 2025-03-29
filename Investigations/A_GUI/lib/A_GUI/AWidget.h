#pragma once

#include <Arduino.h>

#include "ADisplay.h"
#include "AColor.h"
#include "ARect.h"

/**
 * @brief This is the base class for all A_GUI widgets.
 *
 */
class AWidget: virtual public ADisplay {
   public:
    AWidget(void);
    ~AWidget(void);
    static void processTouch(uint16_t xCoord, uint16_t yCoord);  // Process click at specified screen coordinate

   protected:
    static AWidget* allWidgets;                           // Head of the unordered list of all widgets
    AWidget* next;                                        // Next widget in the unordered list of all widgets
    ARect boundary;                                       // Widget boundary rectangle coordinates
    AColor bdColor;                                       // Border color
    AColor bgColor;                                       // Background color
    AColor fgColor;                                       // Foreground color
    void (*doSelection)(ACoord screenX, ACoord screenY);  // Callback function for selected widget
};
