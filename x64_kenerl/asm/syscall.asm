[SECTION .rodata]
enter_syscall_entry: db "cpu %d syscall", 10, 13, 0


global syscall_entry
extern printk
[SECTION .text]
[BITS 64]
syscall_entry:
    swapgs
    mov rsi, [gs:0]
    swapgs

    mov rdi, enter_syscall_entry
    call printk

    ; RCX-返回地址、R11-rflags、rsp需要手动切栈
    jmp $