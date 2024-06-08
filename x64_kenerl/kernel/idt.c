#include "linux/idt.h"
#include "linux/printk.h"
#include "linux/asm.h"

extern void general_interrupt_handler(void);
extern void clock_handler_entry(void);
extern void IPI_TEST_handler_entry(void);
extern void IPI_TEST_handler_entry1(void);
extern void lapic_timer_entry(void);
extern void lapic_sched_broadcast_entry(void);
extern void inter_entry0(void);
extern void inter_entry1(void);
extern void inter_entry2(void);
extern void inter_entry3(void);
extern void inter_entry4(void);
extern void inter_entry5(void);
extern void inter_entry6(void);
extern void inter_entry7(void);
extern void inter_entry8(void);
extern void inter_entry9(void);
extern void inter_entry10(void);
extern void inter_entry11(void);
extern void inter_entry12(void);
extern void inter_entry13(void);
extern void inter_entry14(void);
extern void inter_entry15(void);
extern void inter_entry16(void);
extern void inter_entry17(void);
extern void inter_entry18(void);
extern void inter_entry19(void);
extern void inter_entry20(void);
extern void inter_entry21(void);

static idtr_data_t g_idtr;
static idt_item_t g_idtItems[256];

#define PIC_M_CTRL  0x20    // 主片的控制端口
#define PIC_M_DATA  0x21    // 主片的数据端口
#define PIC_S_CTRL  0xa0    // 从片的控制端口
#define PIC_S_DATA  0xa1    // 从片的数据端口
#define PIC_EOI     0x20    // 通知中断控制器中断结束
void send_eoi(int idt_index) {
    if (idt_index >= 0x20 && idt_index < 0x28) {
        outByte(PIC_M_CTRL, PIC_EOI);
    } else if (idt_index >= 0x28 && idt_index < 0x30) {
        outByte(PIC_M_CTRL, PIC_EOI);
        outByte(PIC_S_CTRL, PIC_EOI);
    }
}

void clock_interrupt_handler() {
    // printk("clock!\n");

    send_eoi(0x20);
}

int init_idt(void)
{
    idt_item_t* item = NULL;
    u64 handler = (u64)general_interrupt_handler;
    for(int i = 0; i < 256; ++i)
    {
        item = &g_idtItems[i];
        switch(i) {
            case 0:
                handler = (u64)inter_entry0;
                break;
            case 1:
                handler = (u64)inter_entry1;
                break;
            case 2:
                handler = (u64)inter_entry2;
                break;
            case 3:
                handler = (u64)inter_entry3;
                break;
            case 4:
                handler = (u64)inter_entry4;
                break;
            case 5:
                handler = (u64)inter_entry5;
                break;
            case 6:
                handler = (u64)inter_entry6;
                break;
            case 7:
                handler = (u64)inter_entry7;
                break;
            case 8:
                handler = (u64)inter_entry8;
                break;
            case 9:
                handler = (u64)inter_entry9;
                break;
            case 10:
                handler = (u64)inter_entry10;
                break;
            case 11:
                handler = (u64)inter_entry11;
                break;
            case 12:
                handler = (u64)inter_entry12;
                break;
            case 13:
                handler = (u64)inter_entry13;
                break;
            case 14:
                handler = (u64)inter_entry14;
                break;
            case 15:
                handler = (u64)inter_entry15;
                break;
            case 16:
                handler = (u64)inter_entry16;
                break;
            case 17:
                handler = (u64)inter_entry17;
                break;
            case 18:
                handler = (u64)inter_entry18;
                break;
            case 19:
                handler = (u64)inter_entry19;
                break;
            case 20:
                handler = (u64)inter_entry20;
                break;
            case 21:
                handler = (u64)inter_entry21;
                break;

            case INTER_ID_TIMER:
                handler = (u64)clock_handler_entry;
                break;
            case INTER_ID_0X80:
                item->dpl = 3;
                break;
            case INTER_ID_IPI_TEST:
                handler = IPI_TEST_handler_entry;
                break;
            case INTER_ID_IPI_TEST1:
                handler = IPI_TEST_handler_entry1;
                break;
            case INTER_ID_LAPIC_TIMER:
                handler = lapic_timer_entry;
                break;
            case INTER_ID_SCHED_BROADCAST:
                handler = lapic_sched_broadcast_entry;
                break;
            default:
                handler = general_interrupt_handler;
                break;
        }
        item->offset0 = handler & 0xffff;
        item->selector = 0x18;
        item->ist = 0;
        item->type = 0b1110; //中断门
        item->segment = 0;
        item->dpl = 0;
        item->present = 1;
        item->offset1 = handler >> 16 & 0xffff;
        item->offset2 = handler >> 32 & 0xffffffff;
    }
    g_idtr.limit = sizeof(g_idtItems) - 1;
    g_idtr.base = (u64)g_idtItems;
    asm volatile ("lidt g_idtr;");
    __asm__("sti;"); //启用中断，置位eflags的IF-[9]
}

int init_ap_idt()
{
    asm volatile ("lidt g_idtr;");
}