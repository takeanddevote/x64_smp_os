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

int print_mem(void *addr, int size, const char *str)
{
    unsigned char *pos = (unsigned char *)addr;
    printk("%s [", str);
    for(int i = 0; i < size; ++i) {
        printk("%02x ", pos[i]);
    }
    printk("].\n");
}
