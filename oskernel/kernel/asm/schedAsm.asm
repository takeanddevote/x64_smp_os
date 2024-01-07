[BITS 32]



[SECTION .text]

extern get_first_sched_flag
extern construct_task_initial_scene
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

.task_fist_sched:
    ;任务第一次调度
    mov esp, [ecx+14*4] ;切换到任务自己的栈
    mov eax, [ecx+8*4]
    push eax ;设置ret的返回地址，设为函数地址即可实现调用任务函数。这里有个细节，push会导致esp变化，而ret会pop，所以前后任务栈不变。
    jmp .siwtch

.task_not_fist_sched:   ;不是第一次调度，则恢复任务现场
    mov eax, [CURRENT]

    mov ebx, [eax+4*11]
    mov ecx, [eax+4*12]
    mov edx, [eax+4*13]
    mov esp, [eax+4*14]
    mov ebp, [eax+4*15]
    mov esi, [eax+4*16]
    mov edi, [eax+4*17]

    push ecx ;先保存ecx

    mov ecx, [eax+4*9] ;恢复eflags，此时eflags是关闭中断标志位状态
    push ecx
    popf ;从栈中恢复值到eflags中

    mov ecx, [eax+4*10]
    mov [esp-4], ecx ;先把eax的值放到栈中
    
    pop ecx;恢复ecx，此时eax在[esp-4]处，并且此时esp和恢复现场的一致。

    mov eax, [eax+4*8]
    push eax ;压栈eip返回地址，配合ret实现任务切换

    mov eax, [esp-8] ;恢复eax

.siwtch:
    sti ;开启中断
    ret


; .switch_task: