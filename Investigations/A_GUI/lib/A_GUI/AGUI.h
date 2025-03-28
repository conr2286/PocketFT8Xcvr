#pragma once

/**
 * A_GUI is a loose collection of GUI-like widgets for an Arduino-like MCU
 *
 * The target environment is a 32-bit MCU with a graphical display supported
 * by derivatives of the Adafruit GFX library.  The original implementation
 * was constructed using:
 *
 *  + Teensy 4.1
 *  + https://www.adafruit.com/product/2050
 *  + HX8357 controller
 *  + https://github.com/mjs513/HX8357_t3n version of Adafruit's GFX library
 *  + HX8357_t3n::getLeading() method added to the GFX library
 */

#include <Arduino.h>

#include "HX8357_t3n.h"

/**
 * Arduino GUI Library
 *
 * This is one header file to #include all of A_GUI
 * 
 */

#include <Adafruit_GFX.h>   //Must #include prior to HX8357
#include <Arduino.h>

#include "ACoord.h"
#include "AColor.h"
#include "AListBox.h"
#include "ARect.h"
#include "HX8357_t3n.h"

