#ifndef __PRINTK_H__
#define __PRINTK_H__
#include "libs/stdarg.h"
#include "logger.h"

int vsprintf(char *buf, const char *fmt, va_list args);
int printk(char *fmt, ...);

#endif /* __PRINTK_H__ */
