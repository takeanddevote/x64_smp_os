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


general_interrupt_msg:
    db "general_interrupt_msg.", 10,13,0