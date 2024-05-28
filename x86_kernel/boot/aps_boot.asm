[ORG 0x9C000]

[SECTION .data]

[SECTION .text]
[BITS 16]
aps_start:
    mov ax, 3
    int 0x10
    jmp $
