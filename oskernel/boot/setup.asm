[ORG 0x500]
[BITS 16]
[SECTION .data]
KERNEL_ADDR equ 0x1200

[SECTION .gdt]
CFG_CODE_SEG_LIMIT equ 0xfffff
CFG_CODE_SEG_BASE equ 0x00
CODE_SECTOR equ (1 << 3)
DATA_SECTOR equ (2 << 3)

db "gdt start.", 10, 13, 0
gdt_base:
    dq 0
gdt_code:
    dw CFG_CODE_SEG_LIMIT & 0xffff  ;代码段的大小限制 G=0则为64KB
    dw CFG_CODE_SEG_BASE & 0xffff   ;代码段base的低16位
    db (CFG_CODE_SEG_BASE >> 16) & 0xff ;代码段base的低16-23位
    db 0b1_00_1_1000 ;段描述符有效、R0特权级、代码段、只允许执行
    db 0b0_1_00_0000 | (CFG_CODE_SEG_LIMIT >> 16) & 0x0f ;limit单位字节、32位的段、limt高4位
    db (CFG_CODE_SEG_BASE >> 24) & 0xff ;代码段base高8位
gdt_data:
    dw CFG_CODE_SEG_LIMIT & 0xffff  ;代码段的大小限制 G=0则为64KB
    dw CFG_CODE_SEG_BASE & 0xffff   ;代码段base的低16位
    db (CFG_CODE_SEG_BASE >> 16) & 0xff ;代码段base的低16-23位
    db 0b1_00_1_0010 ;段描述符有效、R0特权级、数据段段、可读可写
    db 0b1_1_00_0000 | (CFG_CODE_SEG_LIMIT >> 16) & 0x0f ;limit单位字节、32位的段、limt高4位
    db (CFG_CODE_SEG_BASE >> 24) & 0xff ;代码段base高8位
gdt_ptr: ;加载进GDTR寄存器
    dw $ - gdt_base ;低两个字节表示最大偏移
    dd gdt_base ;高四个字节表示GDT表的起始地址
db "gdt end.", 10, 13, 0


[SECTION .text]
global setup_start
setup_start:
    mov     ax, 0   ;把段寄存器清零
    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     si, ax

    mov si, print_jump_success
    call print_str

enter_protected_mode:
    cli ;关中断

    lgdt [gdt_ptr] ;把GDT表和大小偏移加载进GDTR寄存器

    in al, 0x92 ;开A20总线
    or al, 0b00000010
    out 0x92, al

    mov   eax, cr0 ; 设置保护模式
    or    eax , 1
    mov   cr0, eax
    jmp CODE_SECTOR:protected_mode ;跳转到32位模式，这里会同时把代码段选择子设置cs段寄存器


; 如何调用
; mov     si, msg   ; 1 传入字符串
; call    print_str     ; 2 调用
print_str:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
.loop:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10

    inc si
    jmp .loop
.done:
    ret

[BITS 32]
protected_mode:
    ; xchg bx, bx
    mov ax, DATA_SECTOR ;把所有的非代码段寄存器都设置为数据段选择子，实际上数据段和代码段重合的，因为gcc编译的c程序是基于平坦模式的。
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x9fbff ;设置栈顶指针

    mov edi, KERNEL_ADDR
    mov ecx, 3
    mov bl, 50
    call read_hd

    ; xchg bx, bx
    jmp CODE_SECTOR:KERNEL_ADDR


read_hd:
    ; 0x1f2 8bit 指定读取或写入的扇区数
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    ; 0x1f3 8bit iba地址的第八位 0-7
    inc dx
    mov al, cl
    out dx, al

    ; 0x1f4 8bit iba地址的中八位 8-15
    inc dx
    mov al, ch
    out dx, al

    ; 0x1f5 8bit iba地址的高八位 16-23
    inc dx
    shr ecx, 16
    mov al, cl
    out dx, al

    ; 0x1f6 8bit
    ; 0-3 位iba地址的24-27
    ; 4 0表示主盘 1表示从盘
    ; 5、7位固定为1
    ; 6 0表示CHS模式，1表示LAB模式
    inc dx
    shr ecx, 8
    and cl, 0b1111
    mov al, 0b1110_0000     ; LBA模式
    or al, cl
    out dx, al

    ; 0x1f7 8bit  命令或状态端口
    inc dx
    mov al, 0x20
    out dx, al

    ; 设置loop次数，读多少个扇区要loop多少次
    mov cl, bl
.start_read:
    push cx     ; 保存loop次数，防止被下面的代码修改破坏

    call .wait_hd_prepare
    call read_hd_data

    pop cx      ; 恢复loop次数

    loop .start_read

.return:
    ret

; 一直等待，直到硬盘的状态是：不繁忙，数据已准备好
; 即第7位为0，第3位为1，第0位为0
.wait_hd_prepare:
    mov dx, 0x1f7

.check:
    in al, dx
    and al, 0b1000_1000
    cmp al, 0b0000_1000
    jnz .check

    ret

; 读硬盘，一次读两个字节，读256次，刚好读一个扇区
read_hd_data:
    mov dx, 0x1f0
    mov cx, 256

.read_word:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .read_word

    ret



print_jump_success:
    db "second part running.", 10, 13, 0

enter_protected_mode_sucess:
    db "enter protected mode.", 10, 13, 0