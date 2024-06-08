[SECTION .rodata]
enter_syscall_entry: db "cpu %d syscall", 10, 13, 0


global syscall_entry
extern printk
extern system_call_table
[SECTION .text]
[BITS 64]
syscall_entry:

    ; RCX-返回地址、R11-rflags、rsp需要手动切栈
    ; TODO：简单起见，暂时不处理超过6个参数的情况，直接切栈
    ; syscall可以理解为函数调用，既然是函数调用，我们只需遵循调用约定即可，不需要跟中断一样保存现场。
    swapgs
    mov [gs:40], rsp    ; 先保存用户栈
    mov rsp, [gs:48]    ; 切到tss内核栈
    swapgs

    push rcx    ; 保存返回地址、rflags
    push r11

    mov rcx, r10    ; 恢复第四个参数

    lea rax, [rax*8+system_call_table]    ; 调用号计算调用函数地址
    call [rax]

    pop r11     ; 恢复返回地址、rflags
    pop rcx

    swapgs
    mov rsp, [gs:40]    ; 恢复用户栈
    swapgs
    
    o64 sysret  ; o64 - 64位操作模式，不指定会无法返回