#ifndef __PRINTK_H__
#define __PRINTK_H__
#include "lib/stdarg.h"

int print_mem(void *addr, int size, const char *str);
int printk(char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#define debugsit { printk("[%s]", __func__); printk("[%d].\n", __LINE__); }
#endif /* __PRINTK_H__ */