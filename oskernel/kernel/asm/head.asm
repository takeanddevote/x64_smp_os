[SECTION .head.text]
; [ORG 0x1200] ;因为是要编译为可重定位文件，因此链接地址是链接阶段指定的
[BITS 32]

extern kernel_main ;声明C函数
global kernel_start
kernel_start:
    ; xchg bx, bx

    ;初始化中断控制器8259A
.init_8289a:
    mov al, 0x11
    out 0x20, al ;主8259A发送ICW1：级联、IDT描述符8字节、边缘电平触发
    out 0x0a, al ;从8259A发送ICW1

    mov al, 0x20 ;主8259A发送ICW2：中断号从0x20开始
    out 0x21, al
    mov al, 0x28 ;从8259A发送ICW2：中断号从0x28开始
    out 0xa1, al

    mov al, 0x04 ;主8259A发送ICW3：IRQ2线用来级联从片
    out 0x21, al
    mov al, 0x02 ;从8259A发送ICW3：IRQ1线用来级联主片
    out 0xa1, al

    mov al, 0x03 ;主8259A发送ICW4：x86模式、自动EOI
    out 0x21, al
    out 0xa1, al ;从8259A发送ICW4：x86模式、自动EOI

.shield_interrupt:
    mov al, 1111_1100b 
    out 0x21, al ;主8259A发送OCW1：只接受键盘中断，即IRQ1
    mov al, 1111_1111b
    out 0xa1, al ;从8259A发送OCW1：屏蔽所有中断线

    ;调用c语言main函数
.call_c_main:
    call kernel_main
    jmp $
