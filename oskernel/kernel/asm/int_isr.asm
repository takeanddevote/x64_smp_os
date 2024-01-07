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

interrupt_handle:

    push degbug_handle
    call printk
    add esp, 4  ;__cdecl 外平栈

    xchg bx, bx
    iret



extern sched
extern CURRENT
clock_handle:
    ; push degbug_clock_handle
    ; call printk
    ; add esp, 4

    push eax
    mov eax, [CURRENT]
    cmp eax, 0
    je .fisrt_into_tasks

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

    mov eax, esp
    add eax, 12
    mov [ecx+4*14], eax ;保存esp，因为中断push了eflags、cs、eip
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