/*
NAME
  DEBUG.h --- A mock Arduino debugging package for native mode

USAGE EXAMPLES
  #include "DEBUG.h"              //Include this file to enable debugging in a source file
  #include "NODEBUG.h"            //Alternatively include this file to disable debugging in a source file

  DTRACE()                        //Print source filename and line number
  DPRINTF("foo=%u\n",foo)         //Print labeled value of foo

NOTES

LIMITATIONS
  DEBUG_STREAM must support printf (available on Teensy)

LICENSE
  Public domain

ATTRIBUTION
  09/17/2024 Jim Conrad (KQ7B)

*/

#include <Arduino.h>
#pragma once

// Define a symbol such that all debugging messages in all files can be disabled from this one location
#define ENABLE_DEBUG 1  // This should normally be a 1 so each source file can make its own decision

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

//---------------------------------------------------
// The following definitions support ft8_lib debugging
//---------------------------------------------------
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERROR 3
#define LOG_FATAL 4

#ifdef LOG_LEVEL
#ifndef LOG_PRINTF
#include <stdio.h>
#define LOG_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#endif
#define LOG(level, ...)     \
    if (level >= LOG_LEVEL) \
    DPRINTF(__VA_ARGS__)
#else  // ifdef LOG_LEVEL
#define LOG(level, ...)
#endif
