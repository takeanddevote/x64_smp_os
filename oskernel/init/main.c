#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"
#include "linux/idt.h"
#include "linux/memory.h"
#include "linux/vm101012.h"


int kernel_main()
{    
    console_init();
    printk("enter kernel main......\n");
    gdt_init();
    init_idt();
    mm_init();
    vm101012_init(); 

    BOCHS_DEBUG_BREAKPOINT

    while(1); 
}