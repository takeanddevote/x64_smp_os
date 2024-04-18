[SECTION .rodata]
info_not_support_x64: dd "[ERROR] cpu not support x64 mode.", 10, 13, 0
info_x64_vector: dd "cpu vector: %s %s %s.", 10, 13, 0


[SECTION .data]
; 定义局部全局变量
vector_info_1: times 18 db 0x00
vector_info_2: times 18 db 0x00
vector_info_3: times 18 db 0x00



[SECTION .text]
[BITS 32]

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


extern KERNEL_X64_CODE_SECTOR
extern KERNEL_X64_DATA_SECTOR
global enter_x64_mode
enter_x64_mode:
    mov eax, [KERNEL_X64_CODE_SECTOR]
    push eax
    push 0x100000   ; eip x64内核的start address

    ; xor eax, eax
    ; mov eax, [KERNEL_X64_DATA_SECTOR]
    ; mov ds, ax
    ; mov es, ax
    ; mov ss, ax
    ; mov fs, ax
    ; mov gs, ax

    xchg bx,bx
    xchg bx,bx
    retf ; retf 长跳转返回，从栈中弹出：eip、cs
