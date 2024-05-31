#ifndef __CPU_H__
#define __CPU_H__
#include "linux/type.h"

typedef struct {
    uint64_t cpuid;
    uint64_t esp0;
    uint64_t esp3;
    uint64_t thread_esp0;
} kpcr_t;

kpcr_t *kpcr_create(void);
#define kpcr_swapgs()   asm volatile("swapgs;")
static inline uint64_t kpcr_get_offset(uint64_t offset)
{
    asm volatile(
        "mov rax, gs:[rcx];"
        :
        : "c"(offset)
    );
}

uint64_t read_msr(uint32_t msr);
void write_msr(uint32_t msr, uint64_t value);

#endif /* __CPU_H__ */
