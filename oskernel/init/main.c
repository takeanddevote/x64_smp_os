#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"
#include "linux/idt.h"
#include "linux/memory.h"
#include "linux/vm101012.h"
#include "linux/clock.h"


int kernel_main()
{    
    console_init();
    printk("enter kernel main......\n");
    gdt_init();
    init_idt();
    clock_init();
    mm_init();
    vm101012_init();

    BOCHS_DEBUG_BREAKPOINT
    
    void *ptr1 = kmalloc(100);
    void *ptr2 = kmalloc(100);
    void *ptr3 = kmalloc(100);
    void *ptr4 = kmalloc(129);
    printk("ptr1 = %p.\n", ptr1);
    printk("ptr2 = %p.\n", ptr2);
    printk("ptr3 = %p.\n", ptr3);
    printk("ptr4 = %p.\n", ptr4);

    kfree_s(ptr2, 100);
    ptr2 = kmalloc(100);
    printk("ptr2 = %p.\n", ptr2);

    kfree_s(ptr4, 129);
    ptr4 = kmalloc(129);
    printk("ptr4 = %p.\n", ptr4);


    while(1); 
}