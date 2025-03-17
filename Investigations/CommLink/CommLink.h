#pragma once
/**
 * @brief Include definitions required to compile for target environments
 *
 * Arduino:  We get most of what we need from Arduino.h
 *
 * Native:  There are actually many potential native environments and these
 * may require us to mock some Arduino classes.  We handle all that in
 * the NativeStream.h file.
 */

// Configure definitions for the Arduino environment
#ifdef ARDUINO
#include <Arduino.h>

// Configure definitions for the Native environment
#else

// NativeStream defines a Stream class for the Native world
#include "NativeEnviron.h"

#endif