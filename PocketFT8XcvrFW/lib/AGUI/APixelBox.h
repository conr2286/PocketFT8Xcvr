/**
 * APixelBox --- An interactive raster box of pixels
 *
 * @author Jim Conrad, KQ7B
 *
 * @license MIT License
 * @copyright 2025 Jim Conrad, All rights reserved.
 *
 * APixelBox can display indivicual pixels in AWidget
 * AListBox is a AWidget
 *
 * APixelBox uses a callBack function for click events
 *
 */

#pragma once

#include <Arduino.h>

#include "AGUI.h"
#include "AWidget.h"

typedef uint16_t APixelPos;

class APixelBox : public AWidget {
   public:
    // Constructors
    APixelBox(ACoord x1, ACoord y1, APixelPos nRows, APixelPos nCols);
    virtual ~APixelBox() {}

    // Methods
    void drawPixel(APixelPos x, APixelPos y, AColor color);

    // Protected
   protected:
    virtual void touchPixel(APixelPos x, APixelPos y) {}  // Application overrides touchPixel() to receive notifications of touch events

   private:
    void touchWidget(ACoord xScreen, ACoord yScreen) override final;  // We override AWidget touchWidget() to receive touch notifications

    // Private variables
    ACoord r;      // Radius of the rounded corners
    ARect bitmap;  // Bitmap rectangle lies within the boundary
};  // AToggleButton
