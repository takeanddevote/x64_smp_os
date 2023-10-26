#ifndef __PRINTK_H__
#define __PRINTK_H__
#include "lib/stdarg.h"

int printk(char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
#endif /* __PRINTK_H__ */