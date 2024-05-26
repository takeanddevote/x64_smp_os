#ifndef __PAGE_H__
#define __PAGE_H__
#include "linux/type.h"

#define PML4_G_E_ADDR(entry)        (((*((u64*)entry) << 12) >> 24) << 12)  //获取PML4、PDPT、PDT、PTT表项中的地址字段（普通页映射）
#define PML4_VRT_TO_PML4_IDX(vrt)   ((vrt & 0xffffffffffff) >> 39)  //获取虚拟地址的PML4索引字段
#define PML4_VRT_TO_PDPT_IDX(vrt)   ((vrt & 0x7fffffffff) >> 30)    //获取虚拟地址的PDPT索引字段
#define PML4_VRT_TO_PDT_IDX(vrt)    ((vrt & 0x3fffffff) >> 21)      //获取虚拟地址的PDT索引字段
#define PML4_VRT_TO_PDE_IDX(vrt)    ((vrt & 0x1fffff) >> 12)        //获取虚拟地址的PTT索引字段

#endif /* __PAGE_H__ */
