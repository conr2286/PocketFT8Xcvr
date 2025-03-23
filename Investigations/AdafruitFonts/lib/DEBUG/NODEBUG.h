/*
NAME
  NODEBUG.h --- Disables message output from DEBUG.h macros

USAGE
  #include "NODEBUG.h"            //Disable debug output messages

NOTES
  Defines NO-OP versions of the DEBUG.h macros

LICENSE
  Public domain

ATTRIBUTION
  09/17/2024 Jim Conrad (KQ7B)

*/

#define DPRINTF(...) 
#define DFPRINTF(...)
#define DTRACE()   
#define D1TRACE()
#define ASSERT(...)  
