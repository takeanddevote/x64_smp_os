#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "linux/type.h"

#define BOCHS_DEBUG_BREAKPOINT  __asm__("xchg bx, bx");

/* flag == 0 will cause exception */
void assert(u32 flag);

#define CLOSE_INTERRUPT() __asm volatile("cli;")
#define OPEN_INTERRUPT() __asm volatile("sti;")

#endif 
