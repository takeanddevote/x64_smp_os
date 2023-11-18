#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "linux/type.h"
#include "linux/printk.h"
#include "linux/system.h"


typedef struct {
    u32 baseAddrLow; /* 基地址低32位 */
    u32 baseAddrHigh; /* 基地址高32位 */
    u32 lengthAddrLow; /* 内存长度低32位 */
    u32 lengthAddrHigh; /* 内存长度高32位 */
    u32 type; /* 本段内存的类型 1-可用 2-不可用 n-保留 */
}ards_t;

typedef struct {
    u16 ards_num;
    ards_t *ards;
} mem_ards_info_t;


int men_ards_init();

#endif