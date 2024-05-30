#ifndef __IDT_H__
#define __IDT_H__
#include "linux/type.h"

#define INTER_ID_PAGE_FAULT      14

#define INTER_ID_TIMER      0x20
#define INTER_ID_KEYBOARD   0x21
#define INTER_ID_RTC        0x28
#define INTER_ID_IPI_TEST   0xA0


#define INTER_ID_0X80        0x80










typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) idtr_data_t;

typedef struct {
    short offset0;
    short selector;
    char ist;
    char type : 4;    // 任务门/中断门/陷阱门
    char segment : 1; // segment = 0 表示系统段
    char dpl : 2;     // 使用 int 指令访问的最低权限
    char present : 1; // 是否有效
    short offset1;
    int offset2;
    int reserved;
} __attribute__((packed)) idt_item_t;


extern int init_idt(void);
void send_eoi(int idt_index);
extern int init_ap_idt();

#endif /* __IDT_H__ */
