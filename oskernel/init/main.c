#include "linux/console.h"
#include "linux/printk.h"

int kernel_main()
{
    console_init();
    printk("%s.\n", "hello world");
    while(1); 
}