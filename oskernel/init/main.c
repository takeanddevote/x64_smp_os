#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"


int kernel_main()
{

    int a = 0xff;
    a = 0x11;
    
    console_init();
    printk("enter kernel main......\n");
    gdt_init();
    
    BOCHS_DEBUG_BREAKPOINT
    while(1); 
}