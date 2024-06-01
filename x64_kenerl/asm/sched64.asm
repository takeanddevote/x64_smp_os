[SECTION .data]


extern printk
extern get_task_esp0
extern get_task_cs
extern get_task_ss
extern get_task_funtion
extern lapic_send_eoi
global first_sched_task
global sched_task
global task_exit
[SECTION .text]
[BITS 64]
; 不跨态，栈上每个寄存器占8字节：rip cs eflags
; 跨态，栈上每个寄存器占8字节：rip cs eflags rsp ss
first_sched_task:
    ; 第一次调度任务：构造任务退出栈
    swapgs
    mov rdi, [gs:32]
    swapgs
    
    ; 通过cs判断是内核态任务还是用户态任务，二者构造的任务切换返回栈不一样
    ; 对于内核态任务，先切到任务栈，然后在任务栈上构造返回栈，不涉及跨态。而对于用户态任务，则是在当前内核栈上构造返回栈，
    ; 跨态返回用户态时，由cpu切到任务栈，再由用户态构造任务退出栈,然后用户态任务退出函数清理用户态资源，
    ; 再通过系统调用进入内核态清理相关资源，再主动调度其它任务。
    call get_task_cs
    mov rcx, cs
    and rax, 0b111
    and rcx, 0b111
    cmp rax, rcx    ;这里简单判断，如果特权级不相等，则认为任务是用户态任务，因为调度函数是在内核态执行的。
    je .sched_kernel_task

.sched_user_task:

    jmp switch

.sched_kernel_task:
    ; 在qemu下发现即使不跨态，iretq也会弹出5个值，因此得按照跨态来构造返回栈
    call get_task_ss    ; ss
    push rax

    call get_task_esp0  
    mov rcx, task_exit
    mov [rax], rcx
    sub rax, 8

    push rax    ;rsp

    pushfq    ;弹出rflags到栈
    pop rax ;rflags加载到rax
    xor rax, rax
    or rax, 0b1000000000    ;使能中断IF位
    push rax    ; rflags

    call get_task_cs
    push rax    ; cs

    call get_task_funtion
    push rax    ; rip

    call get_task_ss
    mov ds, ax
    ; mov ss, ax
    mov gs, ax
    mov fs, ax

switch:
    call lapic_send_eoi
    iretq

sched_task:
    jmp $


task_exit:
    ; 任务退出后，主动调度任务；如果没有任务调度，则返回原始栈上下文出