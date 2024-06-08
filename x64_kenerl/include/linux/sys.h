#ifndef __SYS_H__
#define __SYS_H__
#include "linux/type.h"

#define system_call_nums    128

extern ssize_t sys_write(int fd, const void *buf, size_t count);

#endif /* __SYS_H__ */
