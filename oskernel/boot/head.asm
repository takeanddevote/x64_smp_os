[SECTION .text]
; [ORG 0x1200] ;nasm编译使用elf或者elf32时，需要在链接器指定链接地址
[BITS 32]

extern kernel_main ;声明C函数
global kernel_start
kernel_start:
    mov eax, 0x99
    call kernel_main
    jmp $



