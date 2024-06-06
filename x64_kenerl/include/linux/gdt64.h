#ifndef __GDT_H__
#define __GDT_H__
#include "linux/type.h"
#include "linux/mm.h"


#define TSS_INDEX 7

typedef struct tss {
    uint32_t reserved0;
    uint64_t rsp0;      // Ring 0 堆栈指针
    uint64_t rsp1;      // Ring 1 堆栈指针（通常不使用）
    uint64_t rsp2;      // Ring 2 堆栈指针（通常不使用）
    uint64_t reserved1;
    uint64_t ist1;      // 中断堆栈表指针1
    uint64_t ist2;      // 中断堆栈表指针2
    uint64_t ist3;      // 中断堆栈表指针3
    uint64_t ist4;      // 中断堆栈表指针4
    uint64_t ist5;      // 中断堆栈表指针5
    uint64_t ist6;      // 中断堆栈表指针6
    uint64_t ist7;      // 中断堆栈表指针7
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base; // I/O 映射基址
} __attribute__((packed)) tss_t;

typedef struct tss_descriptor {
    u16 limit_low;
    u16 base_low;
    u8 base_mid;
    u8 flags1;
    u8 limit_high_and_flags2;
    u8 base_high;
    u32 base_upper;
    u32 reserved;
} __attribute__((packed)) tss_descriptor_t;

typedef struct {
    u16 gdt_max_offset;
    u64 gdt_base_addr;
} __attribute__((packed)) gdtr_value;

void init_tss_current_core();

#endif /* __GDT_H__ */
