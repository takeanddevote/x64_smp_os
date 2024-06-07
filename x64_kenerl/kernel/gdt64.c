#include "linux/gdt64.h"
#include "linux/cpu.h"
#include "libs/string.h"
#include "logger.h"

static gdtr_value *g_gdtr = (gdtr_value *)0x9E000;

static void build_x64_tss_desc(u64 base, tss_descriptor_t *desc)
{
    desc->limit_low = 104;
    desc->base_low = base & 0xFFFF;
    desc->base_mid = (base >> 16) & 0xFF;
    desc->flags1 = 0x89;
    desc->limit_high_and_flags2 = 0;
    desc->base_high = (base >> 24) & 0xFF;
    desc->base_upper = base >> 32;
    desc->reserved = 0;
}

/* 每一个核都需要单独一个tss段选择子、以及单独的tss描述符（多核ltr同一个段选择子会导致如下异常），每个核都需要一个单独的内核栈（不然多核共用一个内核栈导致出错） */

/* 
####################### cpu 1 : Normal exception 13 : code 0x38 #######################
### EIT 0: a software interrupt (INT n, INT3, or INTO) caused the fault.
### TI 0: Fault caused by GDT descriptor.
### Selector index: 7.
 */
void init_tss_current_core()
{
    static int tss_index = TSS_INDEX; //bsp核的tss选择子是7固定的，后续ap核则叠加上去

    tss_t *tss = (tss_t *)kzalloc(sizeof(tss_t));
    tss->rsp0 = (uint64_t)kzalloc(PAGE_SIZE) + PAGE_SIZE;
    debug("tss %p.\n", tss->rsp0);

    tss_descriptor_t *desc = (tss_descriptor_t *)(g_gdtr->gdt_base_addr + g_gdtr->gdt_max_offset);
    build_x64_tss_desc((u64)tss, desc);
    g_gdtr->gdt_max_offset += 16;

    __asm__ volatile ("lgdt %0" : : "m"(*g_gdtr));

    uint16_t tss_selector = tss_index << 3;
    __asm__ volatile ("ltr %0" : : "r"(tss_selector));

    tss_index += 2; //64位下段描述符16字节，即占用两个8位描述符

    //设置内核通用栈到kpcr中，在用户态任务切换中需要访问
    kpcr_swapgs();
    kpcr_set_offset(48, tss->rsp0);
    kpcr_swapgs();
}