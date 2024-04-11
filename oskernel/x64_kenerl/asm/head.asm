[SECTION .rodata]


[SECTION .data]



[SECTION .text]
; [ORG 0x1200] ;因为是要编译为可重定位文件，因此链接地址是链接阶段指定的
[BITS 64]

extern kernel_main ;声明C函数
global kernel_start
kernel_start:
    jmp .text
.text:
    jmp $