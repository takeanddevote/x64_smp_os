#include "driver/tty.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/idt.h"
#include "linux/apic.h"

void x64_ap_main(void)
{
    u64 *ptr = (u64 *)0xA000;
    printk("aps enter c..... 0x%x\n", *ptr);

    while(1);
}


extern char g_printBuffer;
int x64_kernel_main()
{
    console_init();
    mm_init();
    init_idt();
    apic_init();
    ap_init();
    
    debugsit
    while(1);

    return 0;
}

