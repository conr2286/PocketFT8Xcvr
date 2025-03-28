#pragma once

#include <Arduino.h>

#include "AColor.h"
#include "ARect.h"

/**
 * @brief This is the base class for all A_GUI widgets.  
 * 
 */
class AWidget {
   public:
    static void processClick(uint16_t xCoord, uint16_t yCoord);  // Process click at specified screen coordinate

   protected:
    ARect boundary;             // Widget boundary rectangle coordinates
    AColor bdColor;             // Border color
    AColor bgColor;             // Background color
    AColor fgColor;             // Foreground color
    void (*doSelection)(void);  // Callback function for selected widget
};

//Sadly, C++ doesn't allow non-const static initialized class members so we do it the old way here
extern AWidget* allWidgets;     // Head of linked list of all AWidgets