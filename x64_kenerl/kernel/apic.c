#include "linux/apic.h"
#include "linux/type.h"
#include "linux/printk.h"
#include "libs/string.h"

#define EBDA_RSDP_ADDRESS       0x40E
#define EBDA_RSDP_ADDRESS_END   0x80E
#define BDA_RSDP_ADDRESS        0xE0000
#define BDA_RSDP_ADDRESS_END    0x100000
#define RSDP_SIGNATURE          "RSD PTR"

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
    return 0;
}