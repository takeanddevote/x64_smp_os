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
IPI_TEST_handler_entry:
    mov rdi, ipi_test_interrupt_msg
    call printk
    iretq


general_interrupt_msg:
    db "general_interrupt_msg.", 10,13,0

clock_interrupt_msg:
    db "clock_interrupt_msg.", 10,13,0

ipi_test_interrupt_msg:
    db "ipi_test_interrupt_msg.", 10,13,0