[SECTION .rodata]
ipi_test_interrupt_msg:
    db "ipi_test_interrupt_msg apic id %d.", 10,13,0
ipi_test_interrupt_msg1:
    db "ipi_test_interrupt_msg1 apic id %d.", 10,13,0

lapic_timer_interrupt_msg:
    db "lapic_timer_interrupt_msg...", 10,13,0

[SECTION .data]



extern printk

[SECTION .text]
[BITS 64]


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

global lapic_timer_entry
lapic_timer_entry:

    mov rdi, lapic_timer_interrupt_msg
    call printk
    iretq



