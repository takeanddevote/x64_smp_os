#include "linux/apic.h"
#include "linux/type.h"
#include "linux/printk.h"
#include "linux/io.h"
#include "linux/mm.h"
#include "libs/string.h"
#include "logger.h"

#define EBDA_RSDP_ADDRESS       0x40E
#define EBDA_RSDP_ADDRESS_END   0x80E
#define BDA_RSDP_ADDRESS        0xE0000
#define BDA_RSDP_ADDRESS_END    0x100000
#define RSDP_SIGNATURE          "RSD PTR"


#define LAPIC_ID_OFFSET_L       0x20
#define LAPIC_EOI_OFFSET        0xB0
#define LAPIC_SVR_OFFSET        0xF0
#define LAPIC_ICR_OFFSET_L      0x300
#define LAPIC_ICR_OFFSET_H      0x310

#define LAPIC_LVT_TIMER_OFFSET  0x320
#define LAPIC_TIMER_DCR_OFFSET  0x3E0
#define LAPIC_TIMER_ICR_OFFSET  0x380
#define LAPIC_TIMER_CCR_OFFSET  0x390

static apic_info_t g_apicInfo;

static void *get_rsdp_address(const char *data, size_t msize)
{
    int state = 0;
    for(int i = 0; i < msize; ++i) {
        switch(state) {
            case 0:
                if(data[i] == 'R')
                    state = 1;
                break;
            case 1:
                if(data[i] == 'S')
                    state++;
                else
                    state = 0;
                break;
            case 2:
                if(data[i] == 'D')
                    state++;
                else
                    state = 0;
                break;
            case 3:
                if(data[i] == ' ')
                    state++;
                else
                    state = 0;
                break;
            case 4:
                if(data[i] == 'P')
                    state++;
                else
                    state = 0;
                break;
            case 5:
                if(data[i] == 'T')
                    state++;
                else
                    state = 0;
                break;
            case 6:
                if(data[i] == 'R')
                    state++;
                else
                    state = 0;
                break;
        }
        if(state == 7) {
            debugsit
            return &data[i] - 6;
        }
    }
    return NULL;
}

u8 sum(u8 *data, size_t len)
{
    u8 sum = 0;
    for(size_t i = 0; i < len; ++i) {
        sum += data[i];
    }
    return sum;
}

static void print_rsdp_info(rsdp_t *rsdp)
{
    printk("************RSDP************\n");
    printk("checksum: %x tsum: %x.\n", rsdp->checksum, sum((u8*)rsdp, 20));
    printk("OEMID: %s.\n", rsdp->oem_id);
    printk("revision: %d %s.\n", rsdp->revision, rsdp->revision == 0 ? "ACPI 1.0" : "ACPI 2.0");
    printk("rsdtAddress: 0x%x.\n", rsdp->rsdt_address);
    printk("length: %d.\n", rsdp->length);
    printk("xsdtAddress: 0x%x.\n", rsdp->xsdt_address);
    printk("externed checkSum: %x %x.\n", rsdp->extended_checksum, sum((u8*)rsdp, sizeof(rsdp_t)));
    printk("****************************\n");
}

static u32 print_rsdt_info(rsdt_t *rsdt)
{
    printk("************RSDT************\n");
    char signature[5] = {0};
    memcpy(signature, rsdt->signature, 4);
    printk("signature: %s.\n", signature);
    printk("length: %d.\n", rsdt->length);
    printk("revision: %d.\n", rsdt->revision);
    printk("checksum: %d tsum %d.\n", rsdt->checksum, sum((u8*)rsdt, rsdt->length));
    printk("****************************\n");
    return 0;
}

static u32 print_madt_info(madt_t *madt)
{
    printk("************MADT************\n");
    char signature[5] = {0};
    memcpy(signature, madt->signature, 4);
    printk("signature: %s.\n", signature);
    printk("length: %d.\n", madt->length);
    printk("revision: %d.\n", madt->revision);
    printk("checksum: %d tsum %d.\n", madt->checksum, sum((u8*)madt, madt->length));
    printk("****************************\n");
    return 0;
}

static u32 print_interrupt_controller_info(madt_t *madt)
{
    printk("************CONTROLLER************\n");
    printk("local apic address: 0x%x.\n", madt->localInterruptControllerAddress);
    u64 start = (u64)madt->interruptControllerStructure;
    u64 end = madt->length + (u64)madt;
    u64 pos = start;
    local_apic_t *local_apic = NULL;
    io_apic_t *io_apic = NULL;

    g_apicInfo.localInterruptControllerAddress = madt->localInterruptControllerAddress;

    while(1) {
        it_ctrl_header_t *header = (it_ctrl_header_t *)pos;
        switch(header->type) {
            case 0: //local APIC
                local_apic = (local_apic_t *)pos;
                printk("local apic: id %d.\n", local_apic->apicId);
                g_apicInfo.lapic[g_apicInfo.lapic_num++] = *local_apic;
                break;

            case 1: //io APIC
                io_apic = (io_apic_t *)pos;
                g_apicInfo.ioapic[g_apicInfo.ioapic_num++] = *io_apic;
                printk("io apic: id %d.\n", io_apic->ioApicId);
                break;
        }
        pos += (u64)header->length;
        if(pos >= end)
            break;
    }
    printk("****************************\n");
    return 0;
}

uint64_t read_apic_base() {
    uint32_t edx, eax;
    asm volatile ("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1B));
    return ((uint64_t)edx << 32) | (eax & 0xFFFFF000);
}

u32 *get_apic_register_addr(u32 offset)
{
    return (u32 *)(read_apic_base() + offset);
}


int apic_init(void)
{
    rsdp_t *p_rsdp = NULL;
    char str[32] = {0};

    p_rsdp = get_rsdp_address(EBDA_RSDP_ADDRESS, EBDA_RSDP_ADDRESS_END - EBDA_RSDP_ADDRESS);
    if(!p_rsdp) {
        p_rsdp = get_rsdp_address(BDA_RSDP_ADDRESS, BDA_RSDP_ADDRESS_END - BDA_RSDP_ADDRESS);
    }
    if(!p_rsdp) {
        err("find rsdp fail.\n");
        return -1;
    }
    log("find rsdp in %x.\n", p_rsdp);
    print_rsdp_info(p_rsdp);

    if(p_rsdp->revision == 0) {
        if(sum((u8*)p_rsdp, 20) != 0) {
            printk("rsdp check sum fail.\n");
            return -1;
        }
        /* 
            1、检测的rsdt的地址：0x1fe16ee 注意这是物理地址，虚拟地址无所谓。该地址所对应的页表项没有分配，因此需要先建立页表映射才能访问。
                且这个地址所在的页面不能进行缓存，即要写通。
            2、简单起见，0x1fe16ee直接映射到0x1fe16ee上
        */
        void *vrt = ioremap_nocache(p_rsdp->rsdt_address, PAGE_SIZE);
        if(!vrt) {
            err("remap rsdt address 0x%x fail.\n", p_rsdp->rsdt_address);
            return -1;
        }
        rsdt_t *p_rsdt = (rsdt_t *)vrt;
        print_rsdt_info(p_rsdt);

        madt_t *madt = NULL;
        int entryNums = (p_rsdt->length - sizeof(rsdt_t)) / 4;
        for(int i = 0; i < entryNums; ++i) {
            if(!memcmp((void *)p_rsdt->entry[i], "APIC", 4)) {
                madt = (madt_t *)p_rsdt->entry[i];
                break;
            }
        }
        if(!madt) {
            err("find no MADT.\n");
            return -1;  
        }

        // print_madt_info(madt);
        print_interrupt_controller_info(madt);

    } else {
        printk("not support ACPI 2.0.\n");
        return -1;
    }

    return 0;
}

bool check_IPI_success()
{
    u64 base = g_apicInfo.localInterruptControllerAddress;
    lapic_ICR_l_t *icr_val_l = (lapic_ICR_l_t *)(base + LAPIC_ICR_OFFSET_L);
    // debug("status %d.\n", icr_val_l->delivery_status);
    return !icr_val_l->delivery_status;
}

#define APS_BOOI_CODE_ENTRY 0x8000

extern int x64_ap_main(void);
int ap_init(void)
{
    u64 *apJmp = (u64 *)0xA000; //把ap的c入口函数存入内存，ap再读取跳转
    *apJmp = (u64)x64_ap_main;

    //建立虚拟地址映射才能访问
    u64 base = ioremap_nocache(g_apicInfo.localInterruptControllerAddress, PAGE_SIZE); 
    u32 *ICR_L = (u32 *)(base + LAPIC_ICR_OFFSET_L);
    u32 *ICR_H = (u32 *)(base + LAPIC_ICR_OFFSET_H);
    
    lapic_ICR_l_t icr_val_l = {0};
    lapic_ICR_h_t icr_val_h = {0};

    // INIT消息
    icr_val_l.delivery_mode = 0b101;
    icr_val_l.destination_shortland = 0b11;
    *ICR_H = *((u32 *)&icr_val_h);
    *ICR_L = *((u32 *)&icr_val_l);
    if(!check_IPI_success()) {
        err("bsp apic send init IPI fail.\n");
        return -1;
    }

    // SIPI消息
    memset(&icr_val_l, 0, sizeof(icr_val_l));
    icr_val_l.delivery_mode = 0b110;
    icr_val_l.vector = APS_BOOI_CODE_ENTRY >> 12;
    icr_val_l.destination_shortland = 0b11;
    *ICR_H = *((u32 *)&icr_val_h);
    *ICR_L = *((u32 *)&icr_val_l);
    if(!check_IPI_success()) {
        err("bsp apic send SIPI fail.\n");
        return -1;
    }

    // SIPI消息
    memset(&icr_val_l, 0, sizeof(icr_val_l));
    icr_val_l.delivery_mode = 0b101;
    icr_val_l.vector = APS_BOOI_CODE_ENTRY >> 12;
    icr_val_l.destination_shortland = 0b11;
    *ICR_H = *((u32 *)&icr_val_h);
    *ICR_L = *((u32 *)&icr_val_l);
    if(!check_IPI_success()) {
        err("bsp apic send SIPI fail.\n");
        return -1;
    }

    u16 *ap_couts = (u16 *)0x9F000; //内核采用一一映射，物理地址也是0x9FB00
    *ap_couts = 0;

    // 等待所有AP核完成初始化
    while(1) {
        if(*ap_couts < g_apicInfo.lapic_num - 1) {
            // log("waiting aps %d, now %d.\n", g_apicInfo.lapic_num - 1, *ap_couts);
            asm volatile("pause;");
        } else {
            break;
        }
    }
    log("all %d aps work normal... %d.\n", *ap_couts, g_apicInfo.lapic_num - 1);
}

int get_lapic_id()
{
    u32 *apicIdR = get_apic_register_addr(LAPIC_ID_OFFSET_L);
    // debug("ap apic id %d.\n", (*apicIdR) >> 24); //intel不同系列cpu的apic id所在字段不同
    return ((*apicIdR) >> 24);
}

int lapic_send_eoi()
{
    *((u32 *)(read_apic_base() + LAPIC_EOI_OFFSET)) = 0;
    return 0;
}

int ap_local_apic_init(void)
{
    //置位SVR寄存器的bit8来使能本地apic
    u32 *SVR = get_apic_register_addr(LAPIC_SVR_OFFSET);
    *SVR = *SVR | (1 << 8);
    
    return 0;
}

int apic_broadcast_message_interrupt(u8 vector)
{
    u64 base = read_apic_base();
    u32 *ICR_L = (u32 *)(base + LAPIC_ICR_OFFSET_L);
    u32 *ICR_H = (u32 *)(base + LAPIC_ICR_OFFSET_H);
    
    lapic_ICR_l_t icr_val_l = {0};
    lapic_ICR_h_t icr_val_h = {0};

    icr_val_l.vector = vector;
    icr_val_l.delivery_mode = 0b000;
    icr_val_l.trigger_mode = 1;
    icr_val_l.level = 0;
    icr_val_l.destination_shortland = 0b10;
    *ICR_H = *((u32 *)&icr_val_h);
    *ICR_L = *((u32 *)&icr_val_l); 

    if(!check_IPI_success()) {
        err("bsp apic send IPI fail.\n");
        return -1;
    }
    return 0;
}

int lapic_timer_one_shot_start(u8 vector, u32 count)
{
    lapic_lvt_timer_t lvt = {0};
    lvt.vector = vector;
    lvt.mask = 0;
    lvt.timer_mode = 0b00;

    u32 *LVT = get_apic_register_addr(LAPIC_LVT_TIMER_OFFSET);
    u32 *DCR = get_apic_register_addr(LAPIC_TIMER_DCR_OFFSET);
    u32 *ICR = get_apic_register_addr(LAPIC_TIMER_ICR_OFFSET);

    *LVT = *((u32 *)&lvt);
    *DCR = 0x00;
    *ICR = count;

    return 0;
}

int lapic_timer_cycle_start(u8 vector, u32 count)
{
    lapic_lvt_timer_t lvt = {0};
    lvt.vector = vector;
    lvt.mask = 0;
    lvt.timer_mode = 0b01;

    u32 *LVT = get_apic_register_addr(LAPIC_LVT_TIMER_OFFSET);
    u32 *DCR = get_apic_register_addr(LAPIC_TIMER_DCR_OFFSET);
    u32 *ICR = get_apic_register_addr(LAPIC_TIMER_ICR_OFFSET);

    *LVT = *((u32 *)&lvt);
    *DCR = 0x00;
    *ICR = count;

    return 0;
}