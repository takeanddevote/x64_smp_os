#ifndef __TTY_H__
#define __TTY_H__
#include "linux/type.h"

void console_init(void);
void console_write(char *buf, u32 count);

#endif // __TTY_H__
