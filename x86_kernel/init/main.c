#include "linux/console.h"
#include "linux/printk.h"
#include "linux/system.h"
#include "linux/gdt.h"
#include "linux/idt.h"
#include "linux/memory.h"
#include "linux/mm.h"
#include "linux/vm101012.h"
#include "linux/clock.h"
#include "linux/task.h"
#include "lib/unistd.h"
#include "lib/stdio.h"
#include "lib/string.h"
#include "asm/asm.h"
#include "configs/autoconf.h"

#ifdef CONFIG_ARCH_X64
#define FOUR_LEVEL_HEAD_TABLE_ADDR 0x8000
#define FIRST_PDPT_ADDR 0x9000
#define FIRST_PDT_ADDR 0xa000

extern void x64_cpu_check();

static unsigned long lsize = 11;

static void prepare_4level_page_table() {
    // 准备4级头表
    int* four_level_head_table_addr = (int*)FOUR_LEVEL_HEAD_TABLE_ADDR;

    memset(four_level_head_table_addr, 0, 4096);

    *four_level_head_table_addr = FIRST_PDPT_ADDR | 3;
    *(four_level_head_table_addr + 1) = 0;

    // 页目录指针表
    int* pdpt_addr = (int*)FIRST_PDPT_ADDR;

    memset(pdpt_addr, 0, 4096);

    *pdpt_addr = FIRST_PDT_ADDR | 3;
    *(pdpt_addr + 1) = 0;

    // 页目录表
    int* pdt_addr = (int*)FIRST_PDT_ADDR;

    memset(pdt_addr, 0, 4096);

    // 采用2M分页,这里直接填写低端2M内存映射
    *pdt_addr = 0 | 0x83;
    *(pdt_addr + 1) = 0;

    *(pdt_addr + 2) = 0x200000 | 0x83;
    *(pdt_addr + 3) = 0;

    asm volatile("mov cr3, ebx"::"b"(four_level_head_table_addr));
}

static void enter_ia32e() {
    prepare_4level_page_table();

    // 开启物理地址扩展功能：PAE cr4.pae = 1
    asm volatile("mov eax, cr4; bts eax, 5; mov cr4, eax;");

    // 设置型号专属寄存器IA32_EFER.LME，允许IA_32e模式
    asm volatile("mov ecx, 0x0c0000080; rdmsr; bts eax, 8; wrmsr");

    // 开启分页 cr0.pg = 1（必须在做完上面两步才能开启分页，否则会出错
    asm volatile("mov eax, cr0; or eax, 0x80000000; mov cr0, eax;");
}
#endif /* CONFIG_ARCH_X64 */

/* 32位保护模式下，用户态 */
void user_func()
{
    char str[] = "sys call success."; 
    printf("hello user space.\n");
    while(1);
}

/* 32位保护模式下，内核态 */
int kernel_main()
{   
    console_init();
    printk("enter kernel main......\n");
#ifdef CONFIG_ARCH_X64
    check_x64_support();
    enter_ia32e();
    gdt_init();
    if(!check_ia32e_status()) {
        printk("\nEnter IA-32e mode fail.\n");
    }
    BOCHS_DEBUG_BREAKPOINT
    enter_x64_mode();
#endif

    while(1); 
}