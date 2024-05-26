#ifndef __IO_H__
#define __IO_H__

#include "linux/type.h"

void *ioremap_nocache(ptrUint_t phys_addr, unsigned long size);


#endif /* __IO_H__ */
