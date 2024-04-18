#ifndef __IDT_H__
#define __IDT_H__
#include "linux/type.h"

/* 段描述符是小端存储的 */
typedef struct {
    unsigned short isr_offset_0_15;//中断服务程序段内地址偏移低0-15位
    unsigned short segment_selector;//段选择子，即中断服务程序在哪个段里
    unsigned char reserved; //不用

    unsigned char type : 4; //细分段类型
    unsigned char S : 1;    //1-代码段或数据段 0-系统段
    unsigned char DPL : 2;  //访问ISR的最低特权级
    unsigned char P : 1;    //1-段描述符有效 0-无效

    unsigned short isr_offset_16_31;//中断服务程序段内地址偏移低16-31位
} __attribute__((packed)) idt_interrupt_desciptor;


typedef struct {
    u16 idt_max_offset; //idt表的大小-1，注意是整张表的大小，不是表项的多少
    u32 idt_base_addr;
} __attribute__((packed)) idtr_value;

int init_idt();
#endif