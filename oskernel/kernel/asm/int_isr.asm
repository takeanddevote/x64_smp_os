[BITS 32]
[SECTION .text]

extern printk
extern keymap_handler
extern clock_hander


global key_board_handle
global interrupt_handle
global assert_handle
global clock_handle



assert_handle:

    push debug_assert_handle
    call printk
    add esp, 4

    iret

key_board_handle:

    ; push degbug_keyboard_handle
    ; call printk
    ; add esp, 4  ;__cdecl 外平栈

    push 0x21
    call keymap_handler
    add esp, 4

    iret

extern system_call_table
global syscall_handle
syscall_handle:
    ; 1、系统调用导致跨态中断，切tss内核栈并压入ss、esp、eflags、cs、eip。
    ; 需要先保存现场到任务描述符中，处理完系统调用再恢复现场，并iretd返回用户态。
    push eax
    mov eax, [CURRENT]

    mov [eax+4*11], ebx
    mov [eax+4*12], ecx
    mov [eax+4*13], edx
    mov [eax+4*15], ebp
    mov [eax+4*16], esi
    mov [eax+4*17], edi

    mov ecx, eax ;保存eax
    pop eax
    mov [ecx+4*10], eax

    mov eax, [esp] ;保存eip
    mov [ecx+4*8], eax

    mov eax, [esp+4] ;保存cs3
    mov [ecx+4*19], eax

    mov eax, [esp+8] ;保存eflags
    mov [ecx+4*9], eax

    mov eax, [esp+12]  
    mov [ecx+4*14], eax ;保存esp3

    mov eax, [esp+16]
    mov [ecx+4*20], eax ;保存ss3

    ; 2、处理系统调用
    mov ax, ss  ;ss和cs会在跨态时自动设置，其它段寄存器需手动设置。
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; push debug_syscall_handle
    ; call printk
    ; add esp, 4

    mov eax, [CURRENT]
    mov ebx, [eax+4*11] ;之前保存现场已经修改了传参寄存器，因此需要先恢复一下
    mov ecx, [eax+4*12]
    mov edx, [eax+4*13]
    mov eax, [eax+4*10] 
    push edx
    push ecx
    push ebx
    call [system_call_table+eax*4]
    add esp, 12


    ; 3、恢复现场，返回用户态中断处
    mov eax, [CURRENT]

    mov ebx, [eax+4*11]
    ; mov ecx, [eax+4*12]
    mov edx, [eax+4*13]
    ; mov esp, [eax+4*14]
    mov ebp, [eax+4*15]
    mov esi, [eax+4*16]
    mov edi, [eax+4*17]

    ;构造返回栈：ss、esp、eflags、cs、eip
    mov ecx, [eax+20*4]
    push ecx  ;栈设置ss

    mov ecx, [eax+14*4]
    push ecx  ;栈设置esp

    pushf
    mov ecx, [esp]
    add esp, 4
    or ecx, 0b10_00000000 ;开启中断
    push ecx  ;栈设置eflags

    xor ecx, ecx
    mov ecx, [eax+19*4]
    push ecx  ;栈设置cs

    mov ecx, [eax+8*4]
    push ecx  ;栈设置eip

    mov ecx, [eax+4*12] ;恢复ecx
    push ecx
    mov ecx, [eax+4*10] ;恢复eax
    push ecx

    mov ax, [eax+4*20] ;所有选择子都设置为数据段选择子
    mov ds, ax
    ; mov ss, ax ;设置跨态的栈段选择子会异常
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop eax ;因为之前设置了ds了，因此不能再访问内存数据段了，但可以访问栈段。
    pop ecx

    iretd

interrupt_handle:

    push degbug_handle
    call printk
    add esp, 4  ;__cdecl 外平栈

    iret


extern sub_task_counter
extern sched
extern CURRENT
clock_handle:
    ; push degbug_clock_handle
    ; call printk
    ; add esp, 4

    push eax
    mov eax, [CURRENT]
    cmp eax, 0
    je .fisrt_into_tasks    ;第一次进入任务调度

    ;任务的时间片还没执行完，因此不需要进行任务切换，也不需要保存现场，但不能破坏现场，就像普通的一次中断过程。
    push eax
    call sub_task_counter
    cmp eax, 1
    je .task_couter_zero

    ;任务的时间片还没执行完
    add esp, 4
    pop eax
    iret    ;返回到当前任务中断处，继续执行当前任务


.task_couter_zero: ;时间片执行完了，需要调度到下一个高优先级就绪任务
    pop eax

.save_task_scene:;保存任务现场

    mov [eax+4*11], ebx
    mov [eax+4*12], ecx
    mov [eax+4*13], edx
    mov [eax+4*15], ebp
    mov [eax+4*16], esi
    mov [eax+4*17], edi

    mov ecx, eax ;保存eax
    pop eax
    mov [ecx+4*10], eax

    mov eax, [esp] ;保存eip
    mov [ecx+4*8], eax

    mov eax, [esp+8] ;保存eflags
    mov [ecx+4*9], eax

    mov eax, [esp+4] ;保存cs
    mov [ecx+4*19], eax

    xor ebx, ebx
    mov ebx, cs
    and eax, 0b11  
    and ebx, 0b11
    cmp eax, ebx    ;检查是否跨态
    je .not_skip_privilege_level
    
    mov eax, [esp+12]   ;跨态
    mov [ecx+4*14], eax ;保存esp3，跨态：ss、esp、eflags、cs、eip

    mov eax, [esp+16]
    mov [ecx+4*20], eax ;保存ss3
    jmp .deal_hander

.not_skip_privilege_level   ;不跨态的话，打断任务的栈就是esp指向的栈，如果跨态的话，从当前esp寄存器中中取出esp
    mov eax, esp
    add eax, 12
    mov [ecx+4*14], eax ;保存esp0，不跨态：eflags、cs、eip

    xor eax, eax
    mov eax, ss ;保存ss0
    mov [ecx+4*20], eax

    jmp .deal_hander

.fisrt_into_tasks: ;内核第一次进入任务切换，因此不需要保存上个任务的现场。
    pop eax

.deal_hander:
    call clock_hander
    call sched
    iret ;如果还没进入任务切换，那么就还需要返回内核裸机态，因此需要保持栈平衡。

global assert
assert:
    mov eax, [esp+4]
    cmp eax, 0
    jnz .assert_success

.assert_fail:
    int 0x81
    cli
    hlt

.assert_success:
    ret



degbug_clock_handle:
    db "degbug_clock_handle..", 10, 13, 0
degbug_handle:
    db "interrupt_handle..", 10, 13, 0
degbug_keyboard_handle:
    db "degbug_keyboard_handle.", 10, 13, 0

debug_assert_handle:
    db "debug_assert_handle", 10, 13, 0

debug_syscall_handle:
    db "debug_syscall_handle", 10, 13, 0