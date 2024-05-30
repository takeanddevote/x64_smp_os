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
    ; 每10ms发生一次时钟中断
; .set_8253_frequency:
;     mov al, 0x36         ; 设置PIT的工作模式
;     out 0x43, al         ; 发送到控制端口

;     mov ax, 11932        ; 为100Hz计算的除数
;     out 0x40, al         ; 发送低字节到通道0
;     mov al, ah
;     out 0x40, al         ; 发送高字节到通道0

.end_kernel64_main:
    jmp x64_kernel_main