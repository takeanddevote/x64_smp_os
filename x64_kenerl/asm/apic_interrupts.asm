[SECTION .rodata]
ipi_test_interrupt_msg:
    db "ipi_test_interrupt_msg apic id %d.", 10,13,0
ipi_test_interrupt_msg1:
    db "ipi_test_interrupt_msg1 apic id %d.", 10,13,0

lapic_timer_interrupt_msg:
    db "lapic_timer_interrupt_msg...", 10,13,0

lapic_sched_interrupt_msg:
    db "lapic_sched_interrupt_msg...", 10,13,0

capture_no_task_exit:
    db "capture_no_task_exit...", 10,13,0

[SECTION .data]

taskspinlock: dw 0x00000000

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

; 不跨态，栈上每个寄存器占8字节：rip cs eflags
; 跨态，栈上每个寄存器占8字节：rip cs eflags rsp ss
global lapic_timer_entry
lapic_timer_entry:

    mov rdi, lapic_timer_interrupt_msg
    call printk
    call lapic_send_eoi
    iretq


global lapic_sched_broadcast_entry
extern spin_lock
extern spin_unlock
extern spin_lock_init
extern spin_lock_destroy
extern get_next_ready_task
extern set_task_running
extern first_sched_task
extern sched_task
extern get_first_sched_flag
lapic_sched_broadcast_entry:
    push rax
    push rdi

.check_task_runing:
    swapgs
    mov rax, [gs:32]
    swapgs

    cmp rax, 0  ;判断是否有任务在执行
    jne .exit

.seize_spinlock:    ;当前不执行任何任务
    mov rdi, taskspinlock
    call spin_lock

    call get_next_ready_task
    cmp rax, 0  ; 如果不是第一个抢到锁的，则任务可能已经被抢光了,则返回断点处
    je .exit

.save_context:; 抢到任务了,保存上下文到栈中
    push rbx
    push rcx
    push rdx
    ; push rdi
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
    mov cx, ds
    push cx
    mov cx, fs
    push cx
    mov cx, gs
    push cx

    ; 通过压栈的cs和当前的cs比对，判断是否跨态；判断的目的是，如果跨态则不需要保存ss和esp了
    ; 如果不跨态则需要保存ss和esp
    mov rdi, [rsp+18*8+8]
    mov rcx, cs
    and rdi, 0b111
    and rcx, 0b111
    cmp rdi, rcx
    je  .not_cross

    push 1  ;标记跨态
    jmp .save_context_end
    
.not_cross:
    mov rdi, [rsp+40]
    push rdi ;r3 ss
    push 0  ;标记不跨态

.save_context_end:
    ; 把任务指针存进kpcr里；
    swapgs
    mov [gs:32], rax

    mov rdi, rax    ; 设置任务为运行状态
    call set_task_running

    lea rax, [rsp]
    mov [gs:24], rsp ;保存栈rsp;当前rsp保存了上下文

    ; 任务已经抢占结束，可以释放自旋锁了
    mov rdi, taskspinlock
    call spin_unlock

    mov rdi, [gs:32]
    swapgs

    call get_first_sched_flag ;判断是否是第一次调度
    cmp rax, 1
    je .sched_first

    call sched_task
.hlt:
    hlt
    jmp .hlt

.sched_first:
    call first_sched_task
    jmp .hlt

.exit:  ;退出返回断点处
    mov rdi, taskspinlock
    call spin_unlock
    ; mov rdi, lapic_sched_interrupt_msg
    ; call printk
    call lapic_send_eoi

    pop rdi
    pop rax

    mov rdi, capture_no_task_exit
    call printk

    iretq
