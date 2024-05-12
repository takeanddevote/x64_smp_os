#include "linux/idt.h"

extern void general_interrupt_handler(void);

static idtr_data_t g_idtr;
static idt_item_t g_idtItems[256];

int init_idt(void)
{
    idt_item_t* item = NULL;
    for(int i = 0; i < 256; ++i)
    {
        item = &g_idtItems[i];

        item->offset0 = (u64)general_interrupt_handler & 0xffff;
        item->selector = 0x18;
        item->ist = 0;
        item->type = 0b1110; //中断门
        item->segment = 0;
        item->dpl = 0;
        item->present = 1;
        item->offset1 = (u64)general_interrupt_handler >> 16 & 0xffff;
        item->offset2 = (u64)general_interrupt_handler >> 32 & 0xffffffff;
        switch(i) {
            case INTER_ID_TIMER:
                break;
            case INTER_ID_0X80:
                item->dpl = 3;
                break;
            default:
                break;
        }
    }
    g_idtr.limit = 256;
    g_idtr.base = (u64)g_idtItems;
    asm volatile ("lidt g_idtr;");
}