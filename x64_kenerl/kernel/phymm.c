#include "linux/mm.h"
#include "linux/printk.h"
#include "linux/type.h"
#include "libs/bitmap.h"
#include "libs/string.h"

typedef struct {
    u32 baseAddrLow; /* 基地址低32位 */
    u32 baseAddrHigh; /* 基地址高32位 */
    u32 lengthAddrLow; /* 内存长度低32位 */
    u32 lengthAddrHigh; /* 内存长度高32位 */
    u32 type; /* 本段内存的类型 1-可用 2-不可用 n-保留 */
} ards_t;

typedef struct {
    u16 ards_num;
    ards_t *ards;
} mem_ards_info_t;

typedef struct {
    u64 start; //物理内存管理模块的起始地址
    u64 length; //物理内存管理模块的长度

    u8 *bitsMap;    //位图管理区域
    bitmap_t map;
} mm_pages_manager_t;

#define MEN_ARDS_NUM_ADDR   0x1100 //保存ARDS的个数
#define MEN_ARDS_ADDR       0x1102 //保存N个ARDS

/* 内核从0x100000开始，先预留2M，则内存管理模块从0x300000开始 */
#define MEN_BITS_MAP            0x500
#define MEN_KERNEL_START        0x100000
#define MEN_MANAGER_START       0x300000

static mem_ards_info_t g_mem_ards;
static mm_pages_manager_t g_pm;

int mm_init()
{
    /* 获取可用内存信息 */
    g_mem_ards.ards_num = *((u16 *)MEN_ARDS_NUM_ADDR);
    g_mem_ards.ards = (ards_t *)MEN_ARDS_ADDR;

    for(int i = 0; i < g_mem_ards.ards_num; ++i) {
        printk("idx %d: ", i);
        printk("0x%x", g_mem_ards.ards[i].baseAddrHigh);
        printk("%x ", g_mem_ards.ards[i].baseAddrLow);

        printk("0x%x", g_mem_ards.ards[i].lengthAddrHigh);
        printk("%x ", g_mem_ards.ards[i].lengthAddrLow);

        printk("0x%x\n", g_mem_ards.ards[i].type);
    }
    
    for(int i = 0; i < g_mem_ards.ards_num; ++i) {
        // 获取内存块0x100000的长度，作为内存管理模块的区域
        if(MEN_KERNEL_START == ((g_mem_ards.ards[i].baseAddrHigh << 32) | g_mem_ards.ards[i].baseAddrLow)) {
            memset(&g_pm, 0, sizeof(g_pm));
            g_pm.start = MEN_MANAGER_START;
            g_pm.length = ((g_mem_ards.ards[i].lengthAddrHigh << 32) | g_mem_ards.ards[i].lengthAddrLow);

            g_pm.bitsMap = (u8 *)MEN_BITS_MAP;
            bitmap_init(&g_pm.map, g_pm.bitsMap, g_pm.length >> 12, 0);
            break;
        }
    }
}

void *get_free_page()
{
    size_t idx = bitmap_scan(&g_pm.map, 1);
    if(idx == EOF)
        return NULL;

    return g_pm.start + (idx << 12);
}


void free_page(void *p)
{
    size_t idx = ((u64)p - g_pm.start) >> 12;
    bitmap_set(&g_pm.map, idx, 0);
}
