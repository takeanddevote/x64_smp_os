global spin_lock
global spin_unlock
global spin_lock_init
global spin_lock_destroy

[SECTION .text]
[BITS 64]

spin_lock_init:
    xor eax, eax
    mov [rdi], eax
    ret

spin_lock_destroy:
    xor eax, eax
    mov [rdi], eax
    ret

spin_lock:
    mov eax, 1

    xchg eax, [rdi]
    test eax, eax
    jnz .spin_pause

    ret

.spin_pause:
    pause
    jmp spin_lock



spin_unlock:
    mov dword [rdi], 0
    ret


