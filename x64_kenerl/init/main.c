#include "driver/tty.h"
#include "linux/printk.h"


extern char g_printBuffer;
int x64_kernel_main()
{
    console_init();
    // printk("enter x64_kernel_main success.... %d\n", 34);
    // char str[] = "enter x64_kernel_main success...";
    // console_write(str, sizeof(str));

    printk("%d %d %d.\n", 1, 2, 3);
    printk("%d.\n", 1);
    printk("%s %c %d %s %d %d %d %d.\n", "nihao", 49, 3, "buhao", 5, 6, 7, 8);
    while(1);
    

    // testIntCall(1,2,3,4,5,6,7,8);
    // testLongCall((char*)0xfffffffffffffff1, (char*)0xfffffffffffffff2, (char*)0xfffffffffffffff3, (char*)0xfffffffffffffff4,    \
    //     (char*)0xfffffffffffffff5, (char*)0xfffffffffffffff6, (char*)0xfffffffffffffff7, (char*)0xfffffffffffffff8);

    return 0;
}

