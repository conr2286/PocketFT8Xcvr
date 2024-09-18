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

#define DEBUG_STREAM Serial

#define DPRINTF(...) DEBUG_STREAM.printf(__VA_ARGS__ );
#define DTRACE()     DEBUG_STREAM.printf("%s:%u\n",__FILE__,__LINE__);


