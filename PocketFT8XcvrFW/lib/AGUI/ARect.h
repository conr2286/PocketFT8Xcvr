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
#include <SPI.h>
#include <stdint.h>

#include "AGUI.h"

class ARect {
   public:
    // Constructors
    ARect() { x1 = y1 = x2 = y2 = w = h = 0; }
    virtual ~ARect() {}
    ARect(ACoord x, ACoord y, ALength w, ALength h) : x1(x), y1(y), w(w), h(h) {
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
