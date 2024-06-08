#ifndef __CPU_H__
#define __CPU_H__
#include "linux/type.h"
#include "linux/task64.h"

#define IA32_MSR_EFER       0xC0000080
#define IA32_MSR_STAR       0xc0000081
#define IA32_MSR_LSTAR      0xC0000082  
#define IA32_MSR_FMASK      0xc0000084

typedef struct {
    uint64_t cpuid; //lapic id
    uint64_t esp0;  //内核态任务栈
    uint64_t esp3;  //用户态任务栈
    uint64_t stack;  //不运行任务时的原始栈
    task_t *task;   //正在执行的任务
    uint64_t temp; //临时变量，保存上下文中临时存储EIP，避免使用全局变量增加自旋锁
    uint64_t tss_stack; //tss设置的内核栈，在初始化tss时设置。
} kpcr_t;

kpcr_t *kpcr_create(void);
#define kpcr_swapgs()   asm volatile("swapgs;")
static inline uint64_t kpcr_get_offset(uint64_t offset)
{
    asm volatile(
        "mov rax, gs:[rdi];"
    );
}

static inline void kpcr_set_offset(uint64_t offset, uint64_t val) 
{
    asm volatile(
        "mov gs:[rdi], rsi;"
    );
}

uint64_t read_msr(uint32_t msr);
void write_msr(uint32_t msr, uint64_t value);

#endif /* __CPU_H__ */
