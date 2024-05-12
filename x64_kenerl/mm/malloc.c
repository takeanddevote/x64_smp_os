#include "linux/mm.h"
#include "linux/type.h"
#include "libs/string.h"
#include "linux/printk.h"

struct bucket_desc {
    u8 *page; //虚拟地址页空间
    struct bucket_desc *next; //下个桶
    void* freeBlock; //空闲内存块链表
    size_t refCnt; //内存块引用次数
    size_t blockSize; //管理的内存块大小
};

struct bucket_dir {
    size_t size;
    struct bucket_desc *chain;
};

static struct bucket_dir g_bucket_dir[] = {
    {16, NULL},
    {32, NULL},
    {64, NULL},
    {128, NULL},
    {256, NULL},
    {512, NULL},
    {1024, NULL},
    {2048, NULL},
    {4096, NULL},
    {0, NULL},
};

static struct bucket_desc *free_bucket_chain;

struct bucket_desc *construct_free_bucket_chain()
{
    struct bucket_desc *free_chain = (struct bucket_desc *)get_free_page();
    printk("free_chain=%p.\n", free_chain);
    size_t maxIndex = PAGE_SIZE / sizeof(struct bucket_desc);
    int i = 0;
    for(;i < maxIndex - 1; ++i) {
        free_chain[i].next = &free_chain[i+1];
    }
    free_chain[i].next = NULL;
    return free_chain;
}

void *kmalloc(size_t len)
{
    /* 
        1、先遍历桶目录，查看是否能够分配len这么长的内存块。
        2、如果有能力分配，则遍历对应的桶链，寻找空闲的内存块，否则返回NULL。
        3、第一次分配内存块时，没有挂载任何的桶，需要先构造一页空间的空闲桶链，并初始化一个桶，然后插入桶目录中。
     */
    struct bucket_dir *dirPos = g_bucket_dir;
    for(;dirPos->size != 0; ++dirPos) { 
        if(dirPos->size >= len)
            break;
    }
    if(!dirPos->size) {
        printk("Not capable to malloc %d bytes memory block.\n", len);
        return NULL;
    }

    struct bucket_desc *chain = dirPos->chain;
    for(; chain; chain = chain->next) {
        if(chain->freeBlock)
            break;
    }

    if(!chain) { //列表中桶链为空或者桶链的内存块都分配完了。
        if(!free_bucket_chain) { //空闲桶链为空，则插入新创建的桶链。
            free_bucket_chain = construct_free_bucket_chain();
        }

        chain = free_bucket_chain;
        free_bucket_chain = free_bucket_chain->next;
        memset(chain, 0, sizeof(struct bucket_desc));
        chain->blockSize = dirPos->size;

        chain->page = (void *)get_free_page(); //分配内存块页空间
        if(!chain->page)
            return NULL;

        // size_t blockNums = (PAGE_SIZE / chain->blockSize);
        // u8 *pos = chain->page;
        // int i = 0;
        // for(; i < blockNums - 1; ++i) {
        //     *((u8**)(pos + i*chain->blockSize)) = (u8 *)(pos + (i+1)*chain->blockSize);
        // }
        // *((u8**)(pos + i*chain->blockSize)) = NULL;

        size_t blockNums = (PAGE_SIZE / chain->blockSize);
        u32 addr = (u32)chain->page;
        int i = 0;
        for(; i < blockNums - 1; ++i) {
            *((u32 *)(addr + i*chain->blockSize)) = addr + (i+1)*chain->blockSize;
        }
        *((u32 *)(addr + i*chain->blockSize)) = (u32)NULL;

        chain->freeBlock = (void *)chain->page; //指向第一个空闲内存块
        chain->next = dirPos->chain; //新桶头插桶链
        dirPos->chain = chain;
    }

    void *retVal = chain->freeBlock;
    chain->freeBlock = *((u8 **)chain->freeBlock); //下一个空闲内存块
    chain->refCnt++; //引用次数+1

    return retVal;
}

void kfree_s(void *ptr, size_t len)
{
/* 
    1、先根据len找到所属的桶链。
    2、再根据ptr地址，找到所属的桶。
    3、再把内存块插入空闲内存块链表中。
    4、如果桶引用次数为0，即桶的内存块没有被使用，则释放页空间，并从桶链中移除该桶，放回空闲桶链中。
 */
    struct bucket_dir *dirPos = g_bucket_dir;
    for(;dirPos->size != 0; ++dirPos) { 
        if(dirPos->size >= len)
            break;
    }

    if(!dirPos->size) {
        printk("Not found %d bytes memory block.\n", len);
        return;
    }

    struct bucket_desc *chain = dirPos->chain;
    void *pageAlignAddr = (void *)((u32)ptr & ~(0xfff)); //获取内存块所属的页基址
    for(;chain; chain = chain->next) {
        if(chain->page == pageAlignAddr) //通过页基址来匹配桶
            break;
    }

    if(!chain) {
        printk("Not found memory block address %p.\n", ptr);
        return;
    }

    //头插空闲内存链表
    *((u32 *)ptr) = (u32)chain->freeBlock;
    chain->freeBlock = ptr;
    chain->refCnt--;

    /* 桶没有被申请内存块，释放该桶的页空间，并把桶放进空闲桶链 */
    if(!chain->refCnt) {
        free_page(chain->page);
        dirPos->chain = chain->next;

        chain->next = free_bucket_chain;
        free_bucket_chain = chain;
    }
    
}