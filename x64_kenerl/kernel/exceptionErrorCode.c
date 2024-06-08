#include "linux/exceptionErrorCode.h"
#include "linux/printk.h"
#include "linux/apic.h"
#include "logger.h"


void handle_normal_errcode(u32 error_code, u64 *stack, u64 id)
{
    printk("\n####################### cpu %d : Normal exception %d : code 0x%x #######################\n", get_lapic_id(), id, error_code);
    normal_errCode *code = (normal_errCode *)&error_code;

    if (code->EXT) { //External event (bit 0)
        printk("### EIT 1: external event caused the fault.\n");
    } else {
        printk("### EIT 0: a software interrupt (INT n, INT3, or INTO) caused the fault.\n");
            
    }

    if (code->IDT) { //Descriptor location (bit 1)
        printk("### IDT 1: Fault caused by IDT descriptor\n");
    } else {
        if (error_code & 0x4) { //GDT/LDT (bit 2)
            printk("### TI 1: Fault caused by LDT descriptor.\n");
        } else {
            printk("### TI 0: Fault caused by GDT descriptor.\n");
        }
    }
    u16 selector_index = (error_code >> 3) & 0x1FFF;
    printk("### Selector index: %u.\n", selector_index);
    printk("### stack dump %.8p: ", stack);
    print_hex_qword(stack, 6);
}


void handle_PF_errcode(u32 error_code, u64 *stack, u64 id)
{
    PF_errCode *code = (PF_errCode *)&error_code;
    printk("\n####################### cpu %d : Page fault %d : code 0x%x #######################\n", get_lapic_id(), id, error_code);

    if(code->P) {
        printk("### P 1: The fault was caused by a non-present page.\n");
    } else {
        printk("### P 0: The fault was caused by a page-level protection violation.\n");
    }

    if(code->WR) {
        printk("### WR 1: The access causing the fault was a write.\n");
    } else {
        printk("### WR 0: The access causing the fault was a read.\n");
    }

    if(code->US) {
        printk("### US 1: A user-mode access caused the fault.\n");
    } else {
        printk("### US 0: A supervisor-mode access caused the fault.\n");
    }

    if(code->RSVD) {
        printk("### RSVD 1: The fault was caused by a reserved bit set to 1 in some paging-structure entry.\n");
    } else {
        printk("### RSVD 0: The fault was not caused by reserved bit violation.\n");
    }

    if(code->ID) {
        printk("### ID 1: The fault was not caused by an instruction fetch.\n");
    } else {
        printk("### ID 0: The fault was caused by an instruction fetch.\n");
    }

    if(code->PK) {
        printk("### PK 1: The fault was not caused by protection keys.\n");
    } else {
        printk("### PK 0: There was a protection-key violation.\n");
    }

    if(code->SS) {
        printk("### SS 1: The fault was caused by a shadow-stack access.\n");
    } else {
        printk("### SS 0: The fault was not caused by a shadow-stack access.\n");
    }

    if(code->HLAT) {
        printk("### HLAT 1: The fault occurred during HLAT paging.\n");
    } else {
        printk("### HLAT 0: The fault occurred during ordinary paging or due to access rights.\n");
    }

    if(code->SGX) {
        printk("### HLAT 1: The fault resulted from violation of SGX-specific access-control requirements.\n");
    } else {
        printk("### HLAT 0: The fault is not related to SGX.\n");
    }

    printk("### stack dump %.8p: ", stack);
    print_hex_qword(stack+1, 5);
}

void handle_CP_errcode(u32 error_code, u64 *stack, u64 id)
{
    CP_errCode *code = (CP_errCode *)&error_code;
    printk("\n####################### cpu %d : Control protection %d : code 0x%x #######################\n", get_lapic_id(), id, error_code);
    if(code->CPEC == 1) printk("### CPEC 1: The #CP was caused by a near RET instruction.\n");
    if(code->CPEC == 2) printk("### CPEC 2: The #CP was caused by a FAR RET or IRET instruction.\n");
    if(code->CPEC == 3) printk("### CPEC 3: The #CP was due to missing ENDBRANCH at target of an indirect call or jump instruction.\n");
    if(code->CPEC == 4) printk("### CPEC 4: The #CP was caused by a shadow-stack-restore token check failure in the RSTORSSP instruction.\n");
    if(code->CPEC == 5) printk("### CPEC 5: The #CP was caused by a supervisor shadow stack token check failure in the SETSSBSY instruction.\n");
    if(code->ENCL) printk("### ENCL 1: The #CP occurred during enclave execution.\n");

    printk("### stack dump %.8p: ", stack);
    print_hex_qword(stack, 6);
}