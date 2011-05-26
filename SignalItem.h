/* Do not edit this file. It was automatically generated. */

#ifndef HEADER_SignalItem
#define HEADER_SignalItem
/*
htop - SignalItem.h
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "String.h"
#include "Object.h"
#include "RichString.h"
#include <string.h>

#include "debug.h"

#define SIGNAL_COUNT 34


typedef struct Signal_ {
   Object super;
   const char* name;
   int number;
} Signal;


#ifdef DEBUG
extern char* SIGNAL_CLASS;
#else
#define SIGNAL_CLASS NULL
#endif

int Signal_getSignalCount();

Signal** Signal_getSignalTable();

#endif
