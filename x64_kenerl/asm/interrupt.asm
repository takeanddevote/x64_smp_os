[SECTION .data]



global general_interrupt_handler
extern printk
extern print_msg
[SECTION .text]
[BITS 64]
general_interrupt_handler:
    mov rdi, general_interrupt_msg
    call printk

    jmp $
    iretq

extern clock_interrupt_handler
global clock_handler_entry
clock_handler_entry:
    ; mov rdi, clock_interrupt_msg
    ; call printk
    call clock_interrupt_handler
    iretq


global IPI_TEST_handler_entry
global IPI_TEST_handler_entry1
extern lapic_send_eoi
extern get_lapic_id
IPI_TEST_handler_entry:
    call get_lapic_id
    mov rsi, rax
    mov rdi, ipi_test_interrupt_msg
    call printk

    call lapic_send_eoi
    iretq

IPI_TEST_handler_entry1:
    call get_lapic_id
    mov rsi, rax
    mov rdi, ipi_test_interrupt_msg1
    call printk

    call lapic_send_eoi
    iretq


general_interrupt_msg:
    db "general_interrupt_msg.", 10,13,0

clock_interrupt_msg:
    db "clock_interrupt_msg.", 10,13,0

ipi_test_interrupt_msg:
    db "ipi_test_interrupt_msg apic id %d.", 10,13,0

ipi_test_interrupt_msg1:
    db "ipi_test_interrupt_msg1 apic id %d.", 10,13,0