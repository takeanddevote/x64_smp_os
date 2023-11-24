#include "linux/memory.h"
#include "lib/string.h"
#include "linux/system.h"

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
    u32 phy_base; //内存管理的起始物理地址
    u32 phy_pages; //一共管理多少页
    u32 phy_pages_used; //已经分配了多少页
    u32 phy_map_size; //一页使用1B标记，共需多少B
    u8 *phy_map; //映射表
} mm_pages_manager_t;


#define MEN_ARDS_NUM_ADDR   0x1100 //保存ARDS的个数
#define MEN_ARDS_ADDR       0x1102 //保存N个ARDS
#define MEM_VALID       1
#define MEM_INVALID     2


#define MEM_MANAGER_ADDR        0x100000   //内存管理的起始物理地址
#define MEM_MANAGER_MAP_ADD     0x00       //存放映射表的内存地址

static mem_ards_info_t g_mem_ards;
static mm_pages_manager_t g_mm_manager;

void *get_free_page()
{
    void *ret = NULL;
    for(u32 i = 0; i < g_mm_manager.phy_map_size; ++i) {
        if(!g_mm_manager.phy_map[i]) {
            ret = g_mm_manager.phy_base + i*PAGE_SIZE;
            g_mm_manager.phy_map[i] = 1;
            g_mm_manager.phy_pages_used += 1;
            break;
        }
    }
    return ret;
}

void free_page(void *ptr)
{
    assert(ptr != NULL);
    u32 idx = ((u32)(ptr - g_mm_manager.phy_base)) >> 12;
    g_mm_manager.phy_map[idx] = 0;
    g_mm_manager.phy_pages_used--;
}

static int mm_manager_init()
{
    printk("mm physical start: 0x%x\n", g_mm_manager.phy_base);
    printk("mm physical size: %dM\n", (g_mm_manager.phy_pages*4*1024) / (1024*1024));
    printk("mm pages used: %d\n", g_mm_manager.phy_pages_used);
    printk("mm map size: %d\n", g_mm_manager.phy_map_size);
    printk("mm map start: 0x%x\n", g_mm_manager.phy_map);

    g_mm_manager.phy_map = 0x9000; /* 内核在0x1200处，注意不要覆盖了 */
    memset(g_mm_manager.phy_map, 0, g_mm_manager.phy_map_size);
}

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

        if(g_mem_ards.ards[i].baseAddrLow == MEM_MANAGER_ADDR && g_mem_ards.ards[i].type == MEM_VALID) {
            g_mm_manager.phy_base = MEM_MANAGER_ADDR;
            g_mm_manager.phy_pages = g_mem_ards.ards[i].lengthAddrLow / PAGE_SIZE;
            g_mm_manager.phy_pages_used = 0;
            g_mm_manager.phy_map_size = g_mm_manager.phy_pages;
        }

        if(g_mem_ards.ards[i].baseAddrLow == MEM_MANAGER_MAP_ADD && g_mem_ards.ards[i].type == MEM_VALID) {
            g_mm_manager.phy_map = MEM_MANAGER_MAP_ADD;
        }
    }

    if(g_mm_manager.phy_base != MEM_MANAGER_ADDR || g_mm_manager.phy_map != MEM_MANAGER_MAP_ADD) {
        printk("[ERROR] init memory manager fail.\n");
        return -1;
    }

    mm_manager_init();

    return 0;
}