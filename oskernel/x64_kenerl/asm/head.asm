[SECTION .rodata]


[SECTION .data]



[SECTION .head.text]
[BITS 64]

extern kernel_main ;声明C函数
global kernel_start
kernel_start:
    jmp _kernel_start


extern x64_kernel_main
[SECTION .text]
[BITS 64]

_kernel_start:
    jmp x64_kernel_main