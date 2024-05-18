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


general_interrupt_msg:
    db "general_interrupt_msg.", 10,13,0

clock_interrupt_msg:
    db "clock_interrupt_msg.", 10,13,0