[SECTION .data]

USER_X64_DATA_SECTOR    equ (5 << 3) | 0b011;
USER_X64_CODE_SECTOR    equ (6 << 3) | 0b011;
X64_TSS_SECTOR          equ (7 << 3) | 0b11;    //任务段选择子


global store_context_to_stack
global restore_context_from_stack
extern printk
[SECTION .text]
[BITS 64]


store_context_to_stack: ;保存上下文到栈中；
    ; 作为一个函数被call的，因此需要处理压栈的RIP
    push rax
    mov rax, [rsp+8]    ;保存RIP，这里并没有从栈上去除rip，恢复上下文的时候额外会处理。目前栈结构为：rax eip eip cs rflags。。。

    push rdi
    push rbx
    push rcx
    push rdx
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rcx, ds
    push rcx    ; 如果是push一个寄存器，压栈的字节数跟指定寄存器长度有关，比如push ax，则压栈2个字节，push rax则压栈8字节；对于常数push 1则默认压栈8字节
    mov rcx, fs
    push rcx
    mov rcx, gs
    push rcx

    ; 通过压栈的cs和当前的cs比对，判断是否跨态；判断的目的是，如果跨态则不需要保存ss和rsp了，因为跨态已经压栈了ss核rsp
    ; 如果不跨态则需要保存ss和esp
    mov rdi, [rsp+18*8+16]
    mov rcx, cs
    and rdi, 0b111
    and rcx, 0b111
    cmp rdi, rcx
    je  .not_cross

    push 1  ;标记跨态
    jmp .ret
    
.not_cross:
    mov rdi, ss ; 不用清零rdi，这里是把ss赋值给整个rdi，不足的位数补零
    push rdi ;r3 ss
    push 0  ;标记不跨态

.ret:
    push rax ; RIP
    ret


restore_context_from_stack:
    ; 作为一个函数被call的，因此需要处理压栈的RIP
    pop rbx ; 保存RIP

    pop rax ; 出栈跨态标志

    cmp rax, 1  ; 判断跨态标志
    je .cross

    pop rdi ; ss0 ; 不跨态下恢复ss0；跨态则在构造返回栈中cpu自动恢复ss3
    mov ss, di

.cross:
    pop rdi ;gs
    mov gs, di
    pop rdi ;fs
    mov fs, di
    pop rdi ;ds
    mov ds, di
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdx
    pop rdx

    mov [rsp+24], rbx ; 设置rip到ret对应的栈顶位置，目前栈结构为：rbx rdi rax rip rip cs eflags。。。
    pop rbx
    pop rdi
    pop rax
    ;原始栈就保存着返回栈，因此不需要重新构造
    ret


global move_to_user_state:
extern x64_user_main
extern get_task_esp3
move_to_user_state: ; rip、cs、rflags、rsp、ss

    push USER_X64_DATA_SECTOR   ;ss

    swapgs
    mov rdi, [gs:32]
    swapgs
    call get_task_esp3
    push rax    ;rsp
    
    pushfq      ;rflags，当前中断是开着的，因此直接使用当前的rflags

    push USER_X64_CODE_SECTOR   ;cs
    push x64_user_main  ;rip

    mov ax, USER_X64_DATA_SECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    iretq