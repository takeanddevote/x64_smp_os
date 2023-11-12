[BITS 32]
[SECTION .text]

extern printk
extern keymap_handler
global key_board_handle
global interrupt_handle


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

degbug_handle:
    db "interrupt_handle..", 10, 13, 0
degbug_keyboard_handle:
    db "degbug_keyboard_handle.", 10, 13, 0