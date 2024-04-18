#include "linux/vm101012.h"
#include "linux/type.h"
#include "linux/memory.h"
#include "lib/string.h"

 

int vm101012_init()
{
    // 构造内核的虚拟地址空间，线性映射0-4M的物理地址空间-》虚拟地址0-4M，占用第一个pde。
    u32 *pdt = (u32 *)get_free_page();
    memset(pdt, 0, PAGE_SIZE);

    u32 *ptt = (u32 *)get_free_page();
    memset(ptt, 0, PAGE_SIZE);

    for(int i = 0; i < 1024; ++i) { //构造1024个PTE
        u32 *pte = &ptt[i];
        u32 pa = i*PAGE_SIZE;
        *pte = 0x07 | pa;
    }
    // //因为物理内存管理模块是从1M开始的，而内核占用0-4M，因此需要把1-4M的分配给内核。
    // for(int i = 0; i < 766; ++i) { //一页4KB，1M=256页， 3M=768页, 再减去上面分配的两个页=766页
    //     get_free_page();
    // }

    pdt[0] = 0x07 | (u32)ptt;
    asm volatile("mov cr3, eax;" ::"a"((u32)pdt)); //设置CR3，需要在特权级r0设置
    asm volatile("mov eax, cr0;" " or eax, 0x80000000;" " mov cr0, eax"); //置为PG位，使能分页模式
    /* 
        开启分页模式后，不能再使用get_free_page()，因为分配的物理页还没建立映射，因此需要虚拟内存管理模块，来对
        已经映射的但是空闲的虚拟地址进行管理。
     */
}
















