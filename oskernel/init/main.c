#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"
#include "linux/idt.h"
#include "linux/memory.h"
#include "linux/mm.h"
#include "linux/vm101012.h"
#include "linux/clock.h"
#include "linux/task.h"
#include "lib/unistd.h"
#include "lib/stdio.h"
#include "asm/asm.h"

/* 32位保护模式下，用户态 */
void user_func()
{
    char str[] = "sys call success."; 
    printf("hello user space.\n");
    while(1);
}

/* 32位保护模式下，内核态 */
int kernel_main()
{   
    console_init();
    printk("enter kernel main......\n");
    check_x64_support();

    while(1); 
}