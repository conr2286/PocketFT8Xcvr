#pragma once

/**
 * @brief ARect models a rectangle object for A_GUI
 *
 * ARect doesn't draw anthing on the display --- it's just a model.
 *
 * The rectangle is stored with the pixel coordinates of its upper-left
 * and lower-right corners.
 *
 */

#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>

#include "AGUI.h"

class ARect {
   public:
    // Constructors
    ARect() {}
    virtual ~ARect() {}
    ARect(ACoord x1, ACoord y1, ALength w, ALength h) {
        this->x1 = x1;
        this->y1 = y1;
        this->w = w;
        this->h = h;
        this->x2 = x1 + w;
        this->y2 = y1 + h;
    }

    void setCorners(ACoord x1, ACoord y1, ALength w, ALength h);  // Defines the upper-left and lower-right corners of the rectangle

    /**
     * @brief Determines if the specified coordinates lie within this rectangle
     *
     * @param x x-screen coordinate
     * @param y y-screen coordinate
     */
    bool isWithin(ACoord x, ACoord y) const;

    // Purposefully and perhaps foolishly left public
    ACoord x1, y1;  // Upper left corner
    ACoord x2, y2;  // Lower right corner
    ALength w, h;   // Width and height
};
