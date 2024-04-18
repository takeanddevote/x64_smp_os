[BITS 32]



[SECTION .text]

extern sched
extern task_exit
extern get_first_sched_flag
extern CURRENT
global sched_task
sched_task:
    ;判断CURRENT是否为NULL，如果是说明还没有创建任务，直接返回。时钟中断已经保存现场了，因此寄存器可以随便用
    ;但是要注意，此时的栈还是上个任务或者内核栈，不能破坏上个任务的栈内容，就是内容的后面你怎么用都行，也不用考虑栈平衡什么的。
    mov eax, [CURRENT]
    cmp eax, 0
    jne .has_task_to_switch
    ret

.has_task_to_switch:
    ;获取当前的任务描述符，判断是否是第一次调度该任务。
    mov ecx, [CURRENT];因为进入时钟中断时，现场已经保存到任务描述符里，因此寄存器随便用
    push ecx
    call get_first_sched_flag
    ; pop ecx ;此时已经不需要考虑栈平衡，提高效率
    cmp eax, 0
    je .task_not_fist_sched

.task_fist_sched: ; sched_task.task_fist_sched
    ;任务第一次调度
    mov esp, [ecx+14*4] ;设置esp
    mov ax, [ecx+4*20] ;所有选择子都设置为数据段选择子
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, .task_exit ;先压栈任务退出处理函数
    push eax

    ;构造返回栈：eflags、cs、eip
    pushf
    mov eax, [esp]
    add esp, 4
    or eax, 0b10_00000000 ;开启中断
    push eax  ;栈设置eflags

    xor eax, eax
    mov eax, [ecx+19*4]
    push eax  ;栈设置cs

    mov eax, [ecx+8*4]
    push eax  ;栈设置eip
    iret ;TODO：有空更改为iretd

.task_not_fist_sched:   ;不是第一次调度，则恢复任务现场 sched_task.task_not_fist_sched
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


    ; push ecx ;先保存ecx

    ; mov ecx, [eax+4*9] ;恢复eflags。
    ; ;注意保存的eflags的IF位是开启的，因此要清除IF位再恢复，不然在恢复现场就会发生中断，导致程序奔溃。
    ; and ecx, 0b11111111_11111111_11111101_11111111 
    ; push ecx
    ; popf ;从栈中恢复值到eflags中

    ; mov ecx, [eax+4*10]
    ; mov [esp-4], ecx ;先把eax的值放到栈中
    
    ; pop ecx;恢复ecx，此时eax在[esp-4]处，并且此时esp和恢复现场的一致。

    ; mov eax, [eax+4*8]
    ; push eax ;压栈eip返回地址，配合ret实现任务切换

    ; mov eax, [esp-4] ;恢复eax

.siwtch:
    ; sti ;开启中断
    iretd

.task_exit:
    cli ;任务退出后中断处于开启状态，需要关闭确保不被打断。
    mov eax, [CURRENT]
    push eax
    call task_exit

    jmp $ ;不应该运行到这



global construct_test_scene
construct_test_scene: ;构造一个固定的寄存器现场来测试保护现场和恢复现场
    mov eax, 0x01
    mov ecx, 0x02
    mov edx, 0x03
    mov ebx, 0x04
    ; mov esp, 0x1
    mov ebp, esp
    mov esi, 0x05
    mov edi, 0x06

    jmp $

extern sched_by_sleep
global task_deal_sleep
task_deal_sleep:
    push eax
    mov eax, [CURRENT]
    
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

    mov eax, .wakup_return ;eip要设置为.wakup_return
    mov [ecx+4*8], eax

    pushf ;先把eflags压栈
    mov eax, [esp] ;读取eflags
    mov [ecx+4*9], eax
    add esp, 4 ;恢复栈平衡

    mov eax, esp
    mov [ecx+4*14], eax ;保存esp，上面已经栈平衡了，直接保存


    call sched_by_sleep ;主动发起调度

.wakup_return:
    ;因为是先关闭中断，再保存现场，任务描述符里的eflags的中断是关闭的，因此当睡眠任务被唤醒后，中断是关闭状态，因此要主动打开中断。
    sti
    ret


extern USER_CODE_SECTOR
extern USER_DATA_SECTOR
extern get_esp3
extern user_func
global mov_to_user_mode

; ss3、esp3、eflags、cs、eip
mov_to_user_mode:
    mov eax, [CURRENT]

    push eax
    call get_esp3
    add esp, 4

    mov ecx, [USER_DATA_SECTOR]
    push ecx                ;ss3
    push eax                ;esp3
    pushf                   ;eflags

    mov ecx, [USER_CODE_SECTOR]
    push ecx                ;cs
    push call_user_function ;eip

    mov ax, [USER_DATA_SECTOR]    ;手动设置其它段选择子
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    iretd

call_user_function:
    call user_func 
    jmp $