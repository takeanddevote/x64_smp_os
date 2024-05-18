#include "linux/idt.h"
#include "linux/printk.h"
#include "linux/asm.h"

extern void general_interrupt_handler(void);
extern void clock_handler_entry(void);

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
    printk("clock!\n");

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
            case INTER_ID_TIMER:
                handler = (u64)clock_handler_entry;
                break;
            case INTER_ID_0X80:
                item->dpl = 3;
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