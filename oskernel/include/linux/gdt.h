#ifndef __GDT_H__
#define __GDT_H__
#include "linux/type.h"
#include "configs/autoconf.h"

/* 段描述符是小端存储的 */
typedef struct {
    unsigned short segment_limit_0_15;//段大小
    unsigned short base_address_0_15;//段基址
    unsigned char base_address_16_23;

    unsigned char type : 4; //细分S域的段类型
    unsigned char S : 1;    //1-代码段或数据段 0-系统段
    unsigned char DPL : 2;  //该段的特权级权限
    unsigned char P : 1;    //1-段描述符有效 0-无效

    unsigned char segment_limit_16_19 : 4;
    unsigned char AVL : 1;  //用户位
#ifdef CONFIG_ARCH_X64
    unsigned char L : 1;      //x86无效，x64为L位
#else
    unsigned char : 1;      //默认0
#endif
    unsigned char DB: 1;    //0-16位的段 1-32位段
    unsigned char G: 1;     //0-段单位字节 1-段单位4KB

    unsigned char base_address_24_31;
} __attribute__((packed)) gdt_segment_desciptor;


typedef struct {
    u16 gdt_max_offset;
#ifdef CONFIG_ARCH_X64
    u64 gdt_base_addr;
#else
    u32 gdt_base_addr;
#endif
} __attribute__((packed)) gdtr_value;


int gdt_init();

extern u32 KERNEL_CODE_SECTOR;
extern u32 KERNEL_DATA_SECTOR;
#ifdef CONFIG_ARCH_X64
extern u32 KERNEL_X64_CODE_SECTOR;
extern u32 KERNEL_X64_DATA_SECTOR;
#else
extern u32 USER_CODE_SECTOR;
extern u32 USER_DATA_SECTOR;
#endif

#endif