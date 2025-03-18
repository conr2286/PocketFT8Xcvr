/*
NAME
  DEBUG.h --- A simple Arduino debugging package for MPUs sans JTAG

USAGE
  #include "DEBUG.h"              //Include the debug macros in your source file
  #include "NODEBUG.h"            //Disable debug output messages

  DTRACE()                        //Print source filename and line number
  DPRINTF("foo=%u\n",foo)         //Print labeled value of foo

NOTES
  Debug message output is written to a Stream defined by DEBUG_STREAM below

LIMITATIONS
  DEBUG_STREAM must support printf

LICENSE
  Public domain

ATTRIBUTION
  09/17/2024 Jim Conrad (KQ7B)

*/

#include <Arduino.h>
#pragma once

// Define a symbol such that all debugging messages in all files can be disabled at this one location
#define ENABLE_DEBUG 1

#if ENABLE_DEBUG

#define __SOURCEFILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define DEBUG_STREAM Serial

#define DPRINTF(...)                                             \
    {                                                            \
        DEBUG_STREAM.printf("%s:%u ", __SOURCEFILE__, __LINE__); \
        DEBUG_STREAM.printf(__VA_ARGS__);                        \
    }

#define DFPRINTF(...)                                                             \
    {                                                                             \
        DEBUG_STREAM.printf("%s:%u %s ", __SOURCEFILE__, __LINE__, __FUNCTION__); \
        DEBUG_STREAM.printf(__VA_ARGS__);                                         \
    }

#define DTRACE()                                                                   \
    {                                                                              \
        DEBUG_STREAM.printf("%s:%u %s\n", __SOURCEFILE__, __LINE__, __FUNCTION__); \
    }

#define D1PRINTF(...)                                                                \
    {                                                                                \
        static bool D1F__LINE__ = true;                                              \
        if (D1F__LINE__) {                                                           \
            DEBUG_STREAM.printf("%s:%u %s", __SOURCEFILE__, __LINE__, __FUNCTION__); \
            DEBUG_STREAM.printf(__VA_ARGS__);                                        \
            D1F__LINE__ = false;                                                     \
        }                                                                            \
    }

#define D1TRACE()                                                                      \
    {                                                                                  \
        static bool D1T__LINE__ = true;                                                \
        if (D1T__LINE__) {                                                             \
            DEBUG_STREAM.printf("%s:%u %s\n", __SOURCEFILE__, __LINE__, __FUNCTION__); \
            D1T__LINE__ = false;                                                       \
        }                                                                              \
    }

#else
// Disable all debugging in all locations in all files
#include "NODEBUG.h"
#endif
