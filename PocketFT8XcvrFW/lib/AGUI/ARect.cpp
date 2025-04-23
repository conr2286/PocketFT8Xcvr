#include "AGUI.h"
#include "NODEBUG.h"
#include "ARect.h"

/**
 * @brief Define the pixel corners of a rectangle
 * @param x1 Upper-left x-coord
 * @param y1 Upper-left y-coord
 * @param w Width
 * @param h Height
 *
 * Defines the upper-left and lower-right corners of a rectangle using
 * screen coordinates.
 */
void ARect::setCorners(ACoord x1, ACoord y1, ALength w, ALength h) {
    this->x1 = x1;
    this->y1 = y1;
    this->w = w;
    this->h = h;
    this->x2 = x1 + w;
    this->y2 = y1 + h;
    // DPRINTF("boundary.x1=%d, boundary.x2=%d\n", x1, x2);
}

/**
 * @brief Determine if the specified coordinate lies within this ARect
 * @param x x-coord
 * @param y y-coord
 * @return true if coord lies within, else false
 */
bool ARect::isWithin(ACoord x, ACoord y) const {
    DPRINTF("x=%d, y=%d) for (%d,%d) to (%d,%d)\n", x, y, x1,y1,x2,y2);
    if ((x < x1) || (x > x2)) return false;
    if ((y < y1) || (y > y2)) return false;
    DTRACE();
    return true;
}