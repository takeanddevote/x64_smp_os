[BITS 32]


[SECTION .text]

global inByte
global outByte
global inWord
global outWord
inByte:
    push ebp
    mov ebp, esp

    xor eax, eax  ;使用eax返回值，因此要清零

    mov edx, [ebp+8] ;获取端口号传参
    in al, dx

    mov esp, ebp
    pop ebp
    ret

outByte:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8] ;获取第一个参数端口号
    mov eax, [ebp+12] ;获取第二个参数输出值
    out dx, al

    mov esp, ebp
    pop ebp
    ret

inWord:
    push ebp
    mov ebp, esp

    xor eax, eax  ;使用eax返回值，因此要清零

    mov edx, [ebp+8] ;获取端口号传参
    in ax, dx

    mov esp, ebp
    pop ebp
    ret

outWord:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8] ;获取第一个参数端口号
    mov eax, [ebp+12] ;获取第二个参数输出值
    out dx, ax

    mov esp, ebp
    pop ebp
    ret

