#include "lib/unistd.h"

/* 
_syscall3定义write函数，这里有个疑问：buf指向的内存是在用户态，为啥内核能直接访问？
 */
_syscall3(ssize_t,write,int,fd,const void *,buf,size_t,count)