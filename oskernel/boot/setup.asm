[ORG 0x500]
[BITS 16]

[SECTION .text]

global _start
_start:
    mov si, print_jump_success
    call print

    jmp $

print:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
.loop:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10

    inc si
    jmp .loop
.done:
    ret

print_jump_success:
    db "second part running", 10, 13, 0