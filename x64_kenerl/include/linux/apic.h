#ifndef __APIC_H__
#define __APIC_H__
#include "linux/type.h"

typedef struct {
    char     signature[8];       // "RSD PTR "
    uint8_t  checksum;           // Checksum for first 20 bytes
    char     oem_id[6];           // OEM ID
    uint8_t  revision;           // Revision number, 0 for ACPI 1.0, 2 for ACPI 2.0+
    uint32_t rsdt_address;        // Physical address of RSDT (32-bit)
    uint32_t length;             // Length of the RSDP (should be 36 for ACPI 2.0+)
    uint64_t xsdt_address;        // Physical address of XSDT (64-bit)
    uint8_t  extended_checksum;   // Checksum for entire RSDP
    uint8_t  reserved[3];        // Reserved bytes
} __attribute__((packed)) rsdp_t;


typedef struct {
    s8 signature[4];  // "RSDT"
    u32 length;
    u8 revision;
    u8 checksum;
    u8 oemTableId[8];
    u8 oemid[6];
    u8 oemRevision[4];
    u8 creatorId[4];
    u8 creatorRevision[4];
    u32 entry[];   //32位长度的物理地址数组，指向下一级的数据结构入口
} __attribute__((packed)) rsdt_t;

typedef struct {
    s8 signature[4];  // "APIC"
    u32 length;
    u8 revision;
    u8 checksum;
    u8 oemid[6];
    u8 oemTableId[8];
    u8 oemRevision[4];
    u8 creatorId[4];
    u8 creatorRevision[4];
    u32 localInterruptControllerAddress;
    u8 flags[4];
    u8 interruptControllerStructure[];
} __attribute__((packed)) madt_t;


typedef struct {
    u8 type;
    u8 length;
} __attribute__((packed)) it_ctrl_header_t;

typedef struct {
    it_ctrl_header_t header;
    u8 apicUid;
    u8 apicId;
    u32 flags;
} __attribute__((packed)) local_apic_t;

typedef struct {
    it_ctrl_header_t header;
    u8 ioApicId;
    u8 reserve;
    u32 ioApicAddress;
    u32 globalSystemInterruptBase;
} __attribute__((packed)) io_apic_t;

typedef struct {
    u32 localInterruptControllerAddress; //local apic的寄存器物理基地址
    u32 lapic_num;
    local_apic_t lapic[10];
    u32 ioapic_num;
    io_apic_t ioapic[10];
} apic_info_t;

typedef struct {
    u8 vector;

    u8 delivery_mode : 3;
    u8 destination_mode : 1;
    u8 delivery_status : 1;
    u8 reserved1 : 1;
    u8 level : 1;
    u8 trigger_mode : 1;

    u16 reserved2 : 2;
    u16 destination_shortland : 2;
    u16 reserved3 : 12;
} __attribute__((packed)) lapic_ICR_l_t;

typedef struct {
    u16 reserved1;
    u8 reserved2;
    u8 destination_field;
} __attribute__((packed)) lapic_ICR_h_t;

int apic_init(void);
int ap_init(void);

#endif /* __APIC_H__ */
