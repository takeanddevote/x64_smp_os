#include "linux/gdt.h"
#include "linux/printk.h"
#include "linux/type.h"
#include "lib/string.h"
#include "linux/system.h"

static gdt_segment_desciptor g_gdt[256];
static gdtr_value g_gdtr;

u32 KERNEL_CODE_SECTOR = (1 << 3) | 0b00;
u32 KERNEL_DATA_SECTOR = (2 << 3) | 0b00;    //内核态的代码段和数据段描述符
u32 USER_CODE_SECTOR = (3 << 3) | 0b11;
u32 USER_DATA_SECTOR = (4 << 3) | 0b11;    //用户态的代码段和数据段描述符

/* 
1、先使用sgdt指令通过内联汇编把已设置的gdt表地址和表大小读取到全局数组中
2、在全局数组中新增表项
3、再把全局数组的地址和表大小通过sgdt写进GDTR寄存器中
 */

/* 用户态的代码段描述符 */
static void build_user_cs_segment_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0xffff;
    desc->segment_limit_16_19 = 0xf;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //代码段
    desc->type = 0b1000;    //只可执行

    desc->DPL = 0b11;   //R3特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->DB = 1; //32位段
    desc->G = 1; //单位4KB
}

/* 用户态的栈段描述符 */
static void build_user_ss_segment_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0xffff;
    desc->segment_limit_16_19 = 0xf;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //数据段
    desc->type = 0b0010;    //可读可写

    desc->DPL = 0b11;   //R3特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->DB = 1; //32位段
    desc->G = 1; //单位4KB
}

int gdt_init()
{
    __asm__ volatile("sgdt g_gdtr;");
    printk("read gdtr %x ", g_gdtr.gdt_max_offset); /* setup.asm中直接把表的大小作为最大偏移了，注意下 */
    printk("%x.\n", g_gdtr.gdt_base_addr);

    gdt_segment_desciptor *base = (gdt_segment_desciptor *)g_gdtr.gdt_base_addr;
    memcpy(g_gdt, base, g_gdtr.gdt_max_offset);
    // print_mem((void*)base, 24, "des");

    /* 新增段描述符 */
    int idx = 0;
    g_gdtr.gdt_max_offset += 8;
    idx = (g_gdtr.gdt_max_offset / 8) - 1;
    build_user_cs_segment_desc(&g_gdt[idx]);

    g_gdtr.gdt_max_offset += 8;
    idx = (g_gdtr.gdt_max_offset / 8) - 1;
    build_user_ss_segment_desc(&g_gdt[idx]);

    g_gdtr.gdt_base_addr = (u32)g_gdt;
    g_gdtr.gdt_max_offset = g_gdtr.gdt_max_offset;

    __asm__ volatile("lgdt g_gdtr;");
}