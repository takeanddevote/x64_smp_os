#ifndef __PRINTK_H__
#define __PRINTK_H__
#include "libs/stdarg.h"

int vsprintf(char *buf, const char *fmt, va_list args);
int printk(char *fmt, ...);

#define debugsit { printk("[%s]", __func__); printk("[%d].\n", __LINE__); }
#endif /* __PRINTK_H__ */
