#include "linux/memory.h"

#define MEN_ARDS_NUM_ADDR  0x1100 //保存ARDS的个数
#define MEN_ARDS_ADDR 0x1102 //保存N个ARDS

static mem_ards_info_t g_mem_ards;

int men_ards_init()
{
    g_mem_ards.ards_num = *((u16 *)MEN_ARDS_NUM_ADDR);
    g_mem_ards.ards = (ards_t *)MEN_ARDS_ADDR;

    for(int i = 0; i < g_mem_ards.ards_num; ++i) {
        printk("idx %d: ", i);
        printk("0x%x", g_mem_ards.ards[i].baseAddrHigh);
        printk("%x ", g_mem_ards.ards[i].baseAddrLow);

        printk("0x%x", g_mem_ards.ards[i].lengthAddrHigh);
        printk("%x ", g_mem_ards.ards[i].lengthAddrLow);

        printk("0x%x\n", g_mem_ards.ards[i].type);
    }
}

