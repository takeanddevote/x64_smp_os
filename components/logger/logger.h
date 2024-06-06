#ifndef __LOGGER_H__
#define __LOGGER_H__
#include "component_wapper.h"
#include "linux/type.h"

void print_hex(unsigned char *buf, int len);
void print_hex_qword(u64 *buf, int len);
void print_nstring_label(unsigned char *buf, int len, char *label);
void print_nstring(unsigned char *buf, int len);

#define debugsit { p_printf("[%s]", __func__); p_printf("[%d].\n", __LINE__); }

#define OPEN_DEBUG
// #undef OPEN_DEBUG

#ifdef OPEN_DEBUG
    #define info1 "[DEBUG]"
    #define bug(inf, func, line, argv1, ...)    p_printf(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
    #define debug(...)                          bug(info1, __func__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(...) 
#endif /* MY_DEBUG */


#define info2 "[LOG]"
#define _log(inf, func, line, argv1, ...)    p_printf(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
#define log(...)                            _log(info2, __func__, __LINE__, ##__VA_ARGS__)

#define info3 "[WARN]"
#define _warn(inf, func, line, argv1, ...)    p_printf(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
#define warn(...)                            _warn(info3, __func__, __LINE__, ##__VA_ARGS__)

#define info4 "[ERR]"
#define _err(inf, func, line, argv1, ...)    p_printf(inf "[%s" ":" "%d]:" argv1, func, line, ##__VA_ARGS__)
#define err(...)                            _err(info4, __func__, __LINE__, ##__VA_ARGS__)

#endif /* __LOGGER_H__ */
