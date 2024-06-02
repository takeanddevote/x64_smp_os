[SECTION .rodata]
general_interrupt_msg:
    db "general_interrupt_msg.", 10,13,0

clock_interrupt_msg:
    db "clock_interrupt_msg.", 10,13,0

common_interrupt_msg:
    db "######### Exception %d CPU %d #########", 10,13,0

exception_info:
    db "EIP: 0x%x", 10,13,0

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

global inter_entry0
global inter_entry1
global inter_entry2
global inter_entry3
global inter_entry4
global inter_entry5
global inter_entry6
global inter_entry7
global inter_entry8
global inter_entry9
global inter_entry10
global inter_entry11
global inter_entry12
global inter_entry13
global inter_entry14
global inter_entry15
global inter_entry16
global inter_entry17
global inter_entry18
global inter_entry19
extern get_lapic_id
_inter_entry:
    push rsi
    ; swapgs
    ; mov rdi, [gs:0]
    ; swapgs
    call get_lapic_id

    mov rdx, rax
    pop rsi
    mov rdi, common_interrupt_msg
    mov eax, 0
    call printk

    ; mov rcx, [rsp+16]
    ; mov rdx, [rsp+8]
    mov rsi, [rsp]
    mov rdi, exception_info
    mov eax, 0
    call printk

    jmp $
    iretq
inter_entry0:
    mov rsi, 0
    jmp _inter_entry
inter_entry1:
    mov rsi, 1
    jmp _inter_entry
inter_entry2:
    mov rsi, 2
    jmp _inter_entry
inter_entry3:
    mov rsi, 3
    jmp _inter_entry
inter_entry4:
    mov rsi, 4
    jmp _inter_entry
inter_entry5:
    mov rsi, 5
    jmp _inter_entry
inter_entry6:
    mov rsi, 6
    jmp _inter_entry
inter_entry7:
    mov rsi, 7
    jmp _inter_entry
inter_entry8:
    mov rsi, 8
    jmp _inter_entry
inter_entry9:
    mov rsi, 9
    jmp _inter_entry
inter_entry10:
    mov rsi, 10
    jmp _inter_entry
inter_entry11:
    mov rsi, 11
    jmp _inter_entry
inter_entry12:
    mov rsi, 12
    jmp _inter_entry
inter_entry13:
    mov rsi, 13
    jmp _inter_entry
inter_entry14:
    mov rsi, [rsp]
    mov rdi, exception_info
    call printk

    mov rsi, cr2    ; 导致页面错误的线性地址
    mov rdi, exception_info
    call printk

    mov rsi, 14
    jmp _inter_entry
inter_entry15:
    mov rsi, 15
    jmp _inter_entry
inter_entry16:
    mov rsi, 16
    jmp _inter_entry
inter_entry17:
    mov rsi, 17
    jmp _inter_entry
inter_entry18:
    mov rsi, 18
    jmp _inter_entry
inter_entry19:
    mov rsi, 19
    jmp _inter_entry
