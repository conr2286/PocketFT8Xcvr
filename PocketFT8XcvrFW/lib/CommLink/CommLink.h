#pragma once

// Configure definitions for the Arduino environment
#ifdef ARDUINO
#include <Arduino.h>

// Configure definitions for the Native environment
#else

// NativeStream defines a Stream class for the Native world
#include "NativeStream.h"

#endif