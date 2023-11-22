#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"
#include "linux/idt.h"
#include "linux/memory.h"


int kernel_main()
{

    int a = 0xff;
    a = 0x11;
    
    console_init();
    printk("enter kernel main......\n");
    gdt_init();
    init_idt();
    mm_init();
    
    unsigned char *p = get_free_page();
    printk("%x.\n", (u32)p);
    memset(p, 0, PAGE_SIZE);
    free_page(p);

    unsigned char *p1 = get_free_page();
    printk("%x.\n", (u32)p1);
    memset(p1, 0, PAGE_SIZE);
    free_page(p);

    unsigned char *p2 = get_free_page();
    printk("%x.\n", (u32)p2);
    memset(p2, 0, PAGE_SIZE);

    unsigned char *p3 = get_free_page();
    printk("%x.\n", (u32)p3);
    memset(p3, 0, PAGE_SIZE);

    BOCHS_DEBUG_BREAKPOINT

    while(1); 
}