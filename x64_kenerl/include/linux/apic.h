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

int apic_init(void);

#endif /* __APIC_H__ */
