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
    db "capture_no_task_exit %d...", 10,13,0

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
extern store_context_to_stack
extern restore_context_from_stack
lapic_sched_broadcast_entry:
    call store_context_to_stack ; 因为不管走哪条路，都会调用函数，因此需要开始处保存上下文到栈中

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
    jne .seize_task

    ;没抢到任务，释放锁、发送EOI、恢复上下文、退出
    mov rdi, taskspinlock
    call spin_unlock
    jmp .exit

.seize_task:    ;抢到任务了
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

    call lapic_send_eoi
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
    ; debug
    swapgs
    mov rsi, [gs:0]
    swapgs
    mov rdi, capture_no_task_exit
    call printk

    call lapic_send_eoi     ;发送EOI
    call restore_context_from_stack ;恢复上下文
    iretq




; 不跨态，栈上每个寄存器占8字节：rip cs eflags
; 跨态，栈上每个寄存器占8字节：rip cs eflags rsp ss
global lapic_timer_entry
lapic_timer_entry:

    push rax
    push rdi

; 判断当前是否有任务在执行，有则先自减时间片判断是否要切换任务
;   1、需要切换任务，则保存上下文到任务栈中。再判断是否有ready任务可以切换，有则切换ready任务，没有则恢复原始栈并返回。
;   2、不需要切换任务，则恢复栈平衡后，继续执行任务。
; 当前没有任务执行，则判断是否有ready任务，没有直接退出，有则保存上下文到原始栈，再切任务。
.check_task_runing: 
    swapgs
    mov rax, [gs:32]
    swapgs

    cmp rax, 0
    jne .no_task_running

.task_running:


.no_task_running:
    call get_next_ready_task
    cmp rax, 0
    je .exit


.check_ready_task:



.exit:
    pop rdi
    pop rax
    iretq