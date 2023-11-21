#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "linux/type.h"

#define BOCHS_DEBUG_BREAKPOINT  __asm__("xchg bx, bx");



void assert(u32 flag);

#endif 
