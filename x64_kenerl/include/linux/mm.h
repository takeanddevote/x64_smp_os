#ifndef __MM_H__
#define __MM_H__

int mm_init();
void *get_free_page();
void free_page(void *p);

#endif /* __MM_H__ */
