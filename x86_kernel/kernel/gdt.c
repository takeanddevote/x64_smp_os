#include "linux/gdt.h"
#include "linux/printk.h"
#include "linux/type.h"
#include "lib/string.h"
#include "linux/system.h"
#include "linux/task.h"
#include "linux/mm.h"
#include "linux/memory.h"
#include "configs/autoconf.h"

static gdt_segment_desciptor g_gdt[256];
static gdtr_value *g_gdtr = (gdtr_value *)0x9E000;

static tss_t g_tss;

u32 KERNEL_CODE_SECTOR = (1 << 3) | 0b00;
u32 KERNEL_DATA_SECTOR = (2 << 3) | 0b00;    //内核态的代码段和数据段描述符
#ifdef CONFIG_ARCH_X64
u32 KERNEL_X64_CODE_SECTOR = (3 << 3) | 0b00;
u32 KERNEL_X64_DATA_SECTOR = (4 << 3) | 0b00;
u32 USER_X64_CODE_SECTOR = (5 << 3) | 0b011;
u32 USER_X64_DATA_SECTOR = (6 << 3) | 0b011;
u32 X64_TSS_SECTOR = (7 << 3) | 0b11;    //任务段选择子
#endif
u32 USER_CODE_SECTOR = (3 << 3) | 0b11;
u32 USER_DATA_SECTOR = (4 << 3) | 0b11;    //用户态的代码段和数据段描述符
u32 TSS_SECTOR = (5 << 3) | 0b11;    //任务段选择子
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

/* tss任务段描述符 */
static void build_tss_segment_desc(gdt_segment_desciptor *desc)
{
    //设置跨态内核栈。所有用户态进程进入内核态都共用一个内核栈
    g_tss.ss0 = KERNEL_DATA_SECTOR;
    g_tss.esp0 = ((u32)kmalloc(PAGE_SIZE)) + PAGE_SIZE;
    // g_tss.esp0 = ((u32)get_free_page()) + PAGE_SIZE;

    desc->segment_limit_0_15 = 0xffff;
    desc->segment_limit_16_19 = 0xf;

    u32 base = (u32)&g_tss; //段描述符的基址设为tss结构体变量的地址
    desc->base_address_0_15 = base & 0xffff;
    desc->base_address_16_23 = (base >> 16) & 0xff;
    desc->base_address_24_31 = (base >> 24) & 0xff;

    desc->S = 0;    //系统段
    desc->type = 0b1001;    //32bit TSS

    desc->DPL = 0b00;   //R1特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->DB = 1; //32位段
    desc->G = 1; //单位4KB
}

#ifdef CONFIG_ARCH_X64
/* x64 内核态 代码段描述符 */
static void build_x64_r0_cs_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0;
    desc->segment_limit_16_19 = 0;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //代码段
    desc->type = 0b1000;    //只可执行

    desc->DPL = 0b00;   //R0特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->L = 1;    // 64位长模式代码
    desc->DB = 0; // 64位下db必须为0
    desc->G = 1; //单位4KB
}
/* x64 内核态 数据描述符 */
static void build_x64_r0_ds_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0;
    desc->segment_limit_16_19 = 0;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //数据段
    desc->type = 0b0010;    //可读可写

    desc->DPL = 0b00;   //R0特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->L = 1; //64位长模式代码
    desc->DB = 0; //64位下db必须为0
    desc->G = 1; //单位4KB
}

/* x64 用户态 代码段描述符 */
static void build_x64_r3_cs_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0;
    desc->segment_limit_16_19 = 0;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //代码段
    desc->type = 0b1000;    //只可执行

    desc->DPL = 0b11;   //R3特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->L = 1;    // 64位长模式代码
    desc->DB = 0; // 64位下db必须为0
    desc->G = 1; //单位4KB
}
/* x64 用户态 数据描述符 */
static void build_x64_r3_ds_desc(gdt_segment_desciptor *desc)
{
    desc->segment_limit_0_15 = 0;
    desc->segment_limit_16_19 = 0;

    desc->base_address_0_15 = 0;
    desc->base_address_16_23 = 0;
    desc->base_address_24_31 = 0;

    desc->S = 1;    //数据段
    desc->type = 0b0010;    //可读可写

    desc->DPL = 0b11;   //R0特权级
    desc->P = 1;
    desc->AVL = 0;
    desc->L = 1; //64位长模式代码
    desc->DB = 0; //64位下db必须为0
    desc->G = 1; //单位4KB
}

#endif /* CONFIG_ARCH_X64 */

int gdt_init()
{
    __asm__ volatile ("sgdt %0" : "=m"(*g_gdtr));
    printk("read gdtr %x ", g_gdtr->gdt_max_offset); /* setup.asm中直接把表的大小作为最大偏移了，注意下 */
    printk("%x.\n", g_gdtr->gdt_base_addr);

    gdt_segment_desciptor *base = (gdt_segment_desciptor *)g_gdtr->gdt_base_addr;
    memcpy(g_gdt, base, g_gdtr->gdt_max_offset);
    // print_mem((void*)base, 24, "des");

#ifdef CONFIG_ARCH_X64
    int idx = 0;
    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_x64_r0_cs_desc(&g_gdt[idx]); //r0代码段描述符

    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_x64_r0_ds_desc(&g_gdt[idx]); //r0数据段描述符

    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_x64_r3_cs_desc(&g_gdt[idx]); //r3代码段描述符

    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_x64_r3_ds_desc(&g_gdt[idx]); //r3数据段描述符

#else
    /* 新增段描述符 */
    int idx = 0;
    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_user_cs_segment_desc(&g_gdt[idx]); //r3代码段描述符

    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_user_ss_segment_desc(&g_gdt[idx]); //r3数据段描述符

    g_gdtr->gdt_max_offset += 8;
    idx = (g_gdtr->gdt_max_offset / 8) - 1;
    build_tss_segment_desc(&g_gdt[idx]); //tss任务段描述符
#endif /* CONFIG_ARCH_X64 */

    // BOCHS_DEBUG_BREAKPOINT
    // BOCHS_DEBUG_BREAKPOINT
    g_gdtr->gdt_base_addr = (u32)g_gdt;
    g_gdtr->gdt_max_offset = g_gdtr->gdt_max_offset;

    __asm__ volatile ("lgdt %0" : : "m"(*g_gdtr));
    // __asm__ volatile("ltr ax;"::"a"(TSS_SECTOR)); //设置tss段选择子到tr寄存器。注意这里需要先确保tss段描述符已经设置进gdt了，并已经设置gdtr寄存器了（先定义资源），再设置ltr（使用资源）。
}