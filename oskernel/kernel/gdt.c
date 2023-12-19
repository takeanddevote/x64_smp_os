#include "linux/gdt.h"
#include "linux/printk.h"
#include "linux/type.h"
#include "lib/string.h"

static gdt_segment_desciptor g_gdt[256];
static gdtr_value g_gdtr;

/* 
1、先使用sgdt指令通过内联汇编把已设置的gdt表地址和表大小读取到全局数组中
2、在全局数组中新增表项
3、再把全局数组的地址和表大小通过sgdt写进GDTR寄存器中

 */



int gdt_init()
{
    __asm__ volatile("sgdt g_gdtr;");
    printk("read gdtr %x ", g_gdtr.gdt_max_offset); /* setup.asm中直接把表的大小作为最大偏移了，注意下 */
    printk("%x.\n", g_gdtr.gdt_base_addr);

    gdt_segment_desciptor *base = (gdt_segment_desciptor *)g_gdtr.gdt_base_addr;
    memcpy(g_gdt, base, g_gdtr.gdt_max_offset);
    // print_mem((void*)base, 24, "des");

    /* 新增段描述符 */


    g_gdtr.gdt_base_addr = (u32)g_gdt;
    g_gdtr.gdt_max_offset = g_gdtr.gdt_max_offset;

    __asm__ volatile("lgdt g_gdtr;");
}