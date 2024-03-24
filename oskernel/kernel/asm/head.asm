[SECTION .rodata]
info_not_support_x64: dd "[ERROR] cpu not support x64 mode.", 10, 13, 0
info_x64_vector: dd "cpu vector: %s %s %s.", 10, 13, 0


[SECTION .data]
; 定义局部全局变量
vector_info_1: times 18 db 0x00
vector_info_2: times 18 db 0x00
vector_info_3: times 18 db 0x00



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


extern printk
global check_x64_support
check_x64_support:
    ; 先检查是否支持拓展功能号
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000008
    jl .x64_not_support

    ; 检测是否支持长模式
    mov eax, 0x80000001
    cpuid
    bt edx, 29
    jnc .x64_not_support

    ; 检测CPU的品牌信息
    mov eax, 0x80000002
    cpuid
    mov dword [vector_info_1+0*4], eax
    mov dword [vector_info_1+1*4], ebx
    mov dword [vector_info_1+2*4], ecx
    mov dword [vector_info_1+3*4], edx
    mov eax, 0x80000003
    cpuid
    mov dword [vector_info_2+0*4], eax
    mov dword [vector_info_2+1*4], ebx
    mov dword [vector_info_2+2*4], ecx
    mov dword [vector_info_2+3*4], edx
    mov eax, 0x80000004
    cpuid
    mov dword [vector_info_3+0*4], eax
    mov dword [vector_info_3+1*4], ebx
    mov dword [vector_info_3+2*4], ecx
    mov dword [vector_info_3+3*4], edx

    push vector_info_3
    push vector_info_2
    push vector_info_1
    push info_x64_vector
    call printk
    add esp, 4*4
    ret

.x64_not_support:
    push info_not_support_x64
    call printk
    add esp, 4
    jmp $

global check_ia32e_status
check_ia32e_status:
    xor ebx, ebx
    mov ecx, 0xC0000080 ; IA32_EFER寄存器编号
    RDMSR
    bt eax, 10  ;LMA位：检测是否进入ia32e模式
    setc bl
    mov eax, ebx
    ret