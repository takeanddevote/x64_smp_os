#include "linux/io.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/page.h"
#include "libs/string.h"
#include "logger.h"

#define FOUR_LEVEL_HEAD_TABLE_ADDR 0x90000
#define FIRST_PDPT_ADDR 0x91000
#define FIRST_PDT_ADDR 0x92000

static void getPML4Index_2M(ptrUint_t phys_addr, u32 *pml4, u32 *pdpt, u32 *pdt)
{
    *pml4 = PML4_VRT_TO_PML4_IDX(phys_addr);
    *pdpt = PML4_VRT_TO_PDPT_IDX(phys_addr);
    *pdt = PML4_VRT_TO_PDT_IDX(phys_addr);
}

static void *_arch_ioremap_nocache(ptrUint_t align_addr, int cout)
{
    u32 pml4, pdpt, pdt;
    void *vrt = align_addr;

    for(int i = 0; i < cout; ++i) {
        getPML4Index_2M(align_addr, &pml4, &pdpt, &pdt);

        u64 *entry = (u64 *)(FOUR_LEVEL_HEAD_TABLE_ADDR + pml4*8);
        if(*entry == 0) { // 未分配PML4表项
            ptrUint_t page = (ptrUint_t)get_free_page();
            memset((void *)page, 0, PAGE_SIZE);
            *entry = page | 0b1011;
        }
        
        entry = (u64 *)PML4_G_E_ADDR(entry) + pdpt; //PDPTE
        if(*entry == 0) { // 未分配PDPT表项
            ptrUint_t page = (ptrUint_t)get_free_page();
            memset((void *)page, 0, PAGE_SIZE);
            *entry = page | 0b1011;
        }
        entry = (u64 *)PML4_G_E_ADDR(entry) + pdt; //PDE
        *entry = (align_addr) | 0b10001011; // 2M页
        align_addr += 0x200000;
    }

    return vrt;
}

/* 
1、简单起见，一一映射，如果内核高位映射，则加偏移错位映射。
2、目前内核采用2M页映射。
 */
void *ioremap_nocache(ptrUint_t phys_addr, unsigned long size)
{
    u32 mapSize = 0x200000;
    ptrUint_t alignAddr = (phys_addr / mapSize) * mapSize;
    u32 mapCout = size / mapSize + ((size % mapSize) > 0) ? 1 : 0;
    _arch_ioremap_nocache(alignAddr, mapCout);
    return phys_addr;
}
