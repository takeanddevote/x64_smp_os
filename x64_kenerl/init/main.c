#include "driver/tty.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/idt.h"
#include "linux/apic.h"

void x64_ap_main(void)
{
    init_ap_idt(); //和bsp共享idt
    ap_local_apic_init(); //使能本地apic。
    __asm volatile("sti;");

    //初始化专属数据区
    printk("apic %d init suceess.\n", get_lapic_id());
    
    while(1);
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

    // TODO：两个相同的中断消息间隔过短，会出现丢失的情况
    apic_broadcast_message_interrupt(INTER_ID_IPI_TEST);
    delay_s(2);
    apic_broadcast_message_interrupt(INTER_ID_IPI_TEST1);
    debugsit
    delay_s(2);
    lapic_timer_cycle_start(INTER_ID_LAPIC_TIMER, 50000000*10);
    while(1);

    return 0;
}

