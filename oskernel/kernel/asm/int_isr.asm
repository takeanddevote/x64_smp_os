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


clock_handle:
    push degbug_clock_handle
    call printk
    add esp, 4

    call clock_hander

    iret

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