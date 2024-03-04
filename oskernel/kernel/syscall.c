
#include "linux/type.h"
#include "linux/console.h"

ssize_t sys_write(int fd, const void *buf, size_t count)
{
    console_write((char *)buf, count);
    return 0;
}






#define SYSTEM_CALL_NUMS    128

void *system_call_table[SYSTEM_CALL_NUMS] = {
    sys_write,
};