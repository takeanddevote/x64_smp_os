#ifndef __TTY_H__
#define __TTY_H__
#include "linux/type.h"

void console_init(void);
void console_write(char *buf, u32 count);

void init_serial();
void serial_printk(const char *str, u32 count);
void data_out(const char *data, u32 size);

#endif // __TTY_H__
