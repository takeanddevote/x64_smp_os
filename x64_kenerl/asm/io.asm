[SECTION .text]
[BITS 64]

global inByte
global outByte
global inWord
global outWord
inByte: ;edi
    xor rax, rax 
    mov edx, edi
    in al, dx
    ret

outByte: ;edi esi
    mov edx, edi
    mov eax, esi
    out dx, al
    ret

inWord: ;edi
    xor rax, rax 
    mov edx, edi
    in ax, dx
    ret

outWord: ;edi esi
    mov edx, edi
    mov eax, esi
    out dx, ax
    ret


