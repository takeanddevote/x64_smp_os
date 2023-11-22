#include "linux/idt.h"
#include "linux/system.h"
extern void interrupt_handle();
extern void key_board_handle();
extern void assert_handle();

static idt_interrupt_desciptor g_idt_table[255]; //中断最多255个
static idtr_value g_idtr = {
    .idt_max_offset = 0x4321,
    .idt_base_addr = 0x1234,
};

int init_idt()
{
    static int test_static = 0x1ff1;
    //初始化IDT表
    for(int i = 0; i < sizeof(g_idt_table) / sizeof(idt_interrupt_desciptor); ++i) {
        u32 handle = (u32)interrupt_handle;

        switch(i) {
            case 0x21: handle = (u32)key_board_handle; break;
            case 0x81: handle = (u32)assert_handle; break;
            default: break;
        }

        g_idt_table[i].isr_offset_0_15 = handle & 0xffff;
        g_idt_table[i].isr_offset_16_31 = (handle >> 16) & 0xffff;

        g_idt_table[i].segment_selector = 1 << 3; //ISR所在段：代码段、RPL=0

        g_idt_table[i].type = 0b1110;//32位中断门
        g_idt_table[i].S = 0;//系统段
        g_idt_table[i].DPL = 0;//r0级别，即内核态
        g_idt_table[i].P = 1;
    }

    //设置IDTR
    g_idtr.idt_max_offset = sizeof(g_idt_table) / sizeof(idt_interrupt_desciptor) * 8 - 1;
    g_idtr.idt_base_addr = g_idt_table;
    __asm__ volatile ("lidt g_idtr;");

    __asm__("sti;"); //启用中断，置位eflags的IF-[9]
}