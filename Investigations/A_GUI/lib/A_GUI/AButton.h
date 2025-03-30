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
 
 #include "ADisplay.h"
//  #include "AColor.h"
//  #include "ACoord.h"
//  #include "ARect.h"
 #include "AWidget.h"
 
 class AButton : public AWidget {
    public:
 
     // Constructors
     AButton(const char *str, AColor borderColor);    
     AButton(const char *str, ACoord x1, ACoord y1, ACoord w, ACoord h, AColor borderColor);    

    private:
 
 };  // AListBox