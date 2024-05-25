#include "driver/tty.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/idt.h"
#include "linux/apic.h"


extern char g_printBuffer;
int x64_kernel_main()
{
    console_init();
    mm_init();
    init_idt();
    apic_init();

    // void *p1, *p2, *p3;
    // p1 = kmalloc(16);
    // p2 = kmalloc(16);
    // p3 = kmalloc(16);

    // printk("%p %p %p.\n", p1, p2, p3);

    // asm volatile("xchg %bx, %bx; hlt;");
    // int val = 10 / 0;
    debugsit
    while(1);

    return 0;
}

