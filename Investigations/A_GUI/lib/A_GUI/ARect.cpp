#include "AGraphicsDriver.h"

/**
 * @brief Define the pixel corners of a rectangle
 * @param x1 Upper-left x-coord
 * @param y1 Upper-left y-coord
 * @param x2 Lower-right x-coord
 * @param y2 Lower-right x-coord
 *
 * Defines the upper-left and lower-right corners of a rectangle using
 * screen coordinates.
 */
void ARect::setCorners(ACoord x1, ACoord y1, ACoord x2, ACoord y2) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    // DPRINTF("boundary.x1=%d, boundary.x2=%d\n", x1, x2);
}

/**
 * @brief Determine if the specified coordinate lies within this ARect
 * @param x x-coord
 * @param y y-coord
 * @return true if coord lies within, else false
 */
bool ARect::isWithin(ACoord x, ACoord y) {
    DPRINTF("x=%d, y=%d) for (%d,%d) to (%d,%d)\n", x, y, x1,y1,x2,y2);
    if ((x < x1) || (x > x2)) return false;
    if ((y < y1) || (y > y2)) return false;
    DTRACE();
    return true;
}