#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "linux/type.h"

void console_clear();
void console_write(char *buf, u32 count);
void console_init(void);

#endif /* __CONSOLE_H__ */