#include "linux/sys.h"
#include "linux/type.h"
#include "linux/printk.h"
#include "logger.h"


ssize_t sys_write(int fd, const void *buf, size_t count)
{
    debug("fd=%d, buf=%p, count=%d.\n", fd, buf, count);
    return count;
}

void *system_call_table[system_call_nums] = {
    NULL,
    sys_write
};