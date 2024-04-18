#ifndef __MM_H__
#define __MM_H__

#include "linux/type.h"


void *kmalloc(size_t len);
void kfree_s(void *ptr, size_t len);



#endif