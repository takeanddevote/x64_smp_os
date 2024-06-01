#include "linux/cpu.h"
#include "linux/apic.h"
#include "linux/mm.h"
#include "libs/string.h"
#include "logger.h"

#define IA32_GS_BASE            0xC0000101
#define IA32_KERNEL_GS_BASE     0xC0000102


/* 
rdmsr：读取msr寄存器的64位值
ECX：msr寄存器32位地址
EAX：读取的数据的低32位
EDX：读取的数据的高32位
 */
uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;

    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

    return ((uint64_t)high << 32) | low;
}

/* 
wrmsr：写入msr寄存器的64位值
ECX：msr寄存器32位地址
EAX：读取的数据的低32位
EDX：读取的数据的高32位
 */
void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);

    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

kpcr_t *g_kpcrs[100];    //调试使用

kpcr_t *kpcr_create(void)
{
    int cpuid = get_lapic_id();
    kpcr_t *kpcr = (kpcr_t *)kzalloc(sizeof(kpcr_t));
    if(!kpcr) {
        err("cpu %d create kpcr fail.\n", cpuid);
        return NULL;
    }
    
    kpcr->cpuid = cpuid;
    kpcr->stack = (uint64_t)kzalloc(PAGE_SIZE);

    write_msr(IA32_KERNEL_GS_BASE, (uint64_t)kpcr);
    
    g_kpcrs[cpuid] = kpcr;
    return kpcr;
}

