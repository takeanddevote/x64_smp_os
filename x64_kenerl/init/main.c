#include "driver/tty.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/idt.h"
#include "linux/apic.h"
#include "linux/cpu.h"

void x64_ap_main(void)
{
    init_ap_idt(); //和bsp共享idt
    ap_local_apic_init(); //使能本地apic。
    __asm volatile("sti;");

    kpcr_create();
    kpcr_swapgs();
    int cpuid = kpcr_get_offset(0);
    kpcr_swapgs();

    //初始化专属数据区
    printk("ap kpcr %d init suceess..\n", cpuid);
    
    while(1) {
        asm volatile("hlt;");
        // kpcr_swapgs();
        // log("cores %d wait up from hlt....\n", kpcr_get_offset(0));
        // kpcr_swapgs();
    }
}

void _delay_ms()
{
    for(int i = 0; i < 100; ++i) {
        for(int j = 0; j < 10000;) {
            ++j;
        }
    }
}
void delay_s(int ms)
{
    ms = ms*10;
    while(ms-- >0) {
        _delay_ms();
    }
}

int x64_kernel_main()
{
    console_init();
    mm_init();
    init_idt();
    apic_init();
    ap_init();

    lapic_timer_cycle_start(INTER_ID_LAPIC_TIMER, 50000000*10);
    while(1) {
        asm volatile("hlt;");
        // kpcr_swapgs();
        // log("cores %d wait up from hlt....\n", kpcr_get_offset(0));
        // kpcr_swapgs();
    }

    return 0;
}

