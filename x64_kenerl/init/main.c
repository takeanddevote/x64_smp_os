#include "driver/tty.h"
#include "linux/printk.h"
#include "linux/mm.h"


extern char g_printBuffer;
int x64_kernel_main()
{
    console_init();
    mm_init();

    void *p1 = get_free_page();
    void *p2 = get_free_page();
    void *p3 = get_free_page();
    printk("%p.\n", p1);
    printk("%p.\n", p2);
    printk("%p.\n", p3);

    free_page(p2);
    p2 = get_free_page();
    printk("%p.\n", p2);

    while(1);

    return 0;
}

