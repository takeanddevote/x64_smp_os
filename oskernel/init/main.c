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
    men_ards_init();

    BOCHS_DEBUG_BREAKPOINT
    __asm__("sti;"); //启用中断，置位eflags的IF-[9]

    while(1); 
}