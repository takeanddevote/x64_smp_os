[SECTION .data]


extern printk
global first_sched_task
global sched_task
global task_exit
[SECTION .text]
[BITS 64]
; 不跨态，栈上每个寄存器占8字节：rip cs eflags
; 跨态，栈上每个寄存器占8字节：rip cs eflags rsp ss
first_sched_task:
    ; 第一次调度任务：构造任务退出栈
    jmp $
    swapgs

    swapgs

    iretd

sched_task:
    jmp $




task_exit:
    ; 任务退出后，主动调度任务；如果没有任务调度，则返回原始栈上下文出