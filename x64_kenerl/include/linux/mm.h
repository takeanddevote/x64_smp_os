#ifndef __MM_H__
#define __MM_H__
#include "linux/type.h"

#define PAGE_SIZE   4096

int mm_init();
void *get_free_page();
void free_page(void *p);


void *kmalloc(size_t len);
void kfree_s(void *ptr, size_t len);

#endif /* __MM_H__ */
