#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "linux/type.h"
#include "linux/printk.h"
#include "linux/system.h"

#define PAGE_SIZE 4096

int mm_init();
void *get_free_page();
void free_page(void *ptr);

#endif