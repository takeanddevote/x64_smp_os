#ifndef __PRINTK_H__
#define __PRINTK_H__
#include "libs/stdarg.h"

int vsprintf(char *buf, const char *fmt, va_list args);
int printk(char *fmt, ...);
void print_hex(unsigned char *buf, int len);


#define debugsit { printk("[%s]", __func__); printk("[%d].\n", __LINE__); }

#define OPEN_DEBUG
// #undef OPEN_DEBUG

#ifdef OPEN_DEBUG
    #define info1 "[DEBUG]"
    #define bug(inf, func, line, argv1, ...)    printk(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
    #define debug(...)                          bug(info1, __func__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(...) 
#endif /* MY_DEBUG */


#define info2 "[LOG]"
#define _log(inf, func, line, argv1, ...)    printk(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
#define log(...)                            _log(info2, __func__, __LINE__, ##__VA_ARGS__)

#define info3 "[ERR]"
#define _err(inf, func, line, argv1, ...)    printk(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
#define err(...)                            _err(info2, __func__, __LINE__, ##__VA_ARGS__)

#endif /* __PRINTK_H__ */
