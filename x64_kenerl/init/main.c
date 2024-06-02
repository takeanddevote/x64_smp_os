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

    kpcr_create();
    kpcr_swapgs();
    int cpuid = kpcr_get_offset(0);
    uint64_t stack = kpcr_get_offset(24);
    asm volatile(
        "mov rsp, rax;" 
        :: "a"(stack)
    );
    kpcr_swapgs();
    __asm volatile("sti;"); //必须设置好栈再开中断，不然广播调度消息时，多ap有几率共用一个栈，导致异常


    printk("ap kpcr %d stack %x init suceess..\n", cpuid, stack);

    //初始化专属数据区
    
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
    task_init();

    delay_s(1);
    apic_broadcast_message_interrupt(INTER_ID_SCHED_BROADCAST);

    while(1) {
        asm volatile("hlt;");
        // kpcr_swapgs();
        // log("cores %d wait up from hlt....\n", kpcr_get_offset(0));
        // kpcr_swapgs();
    }

    return 0;
}

