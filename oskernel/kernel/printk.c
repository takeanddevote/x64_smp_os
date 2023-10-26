#include "linux/printk.h"
#include "linux/console.h"

static char buff[1024];
int printk(char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = vsprintf(buff, fmt, args);
    va_end(args);
    console_write(buff, ret);
    return ret;
}