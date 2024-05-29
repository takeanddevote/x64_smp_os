[ORG 0x8000]
;注意链接地址不要超过0xffff，不然跳转实模式下jmp时是长跳转，对跳转地址不太好处理，因为地址超过16位了。

[SECTION .gdt]
CODE_SECTOR equ (1 << 3)
DATA_SECTOR equ (2 << 3)


[SECTION .data]
ap_boot_lock: dd 0x0000   ;自旋锁变量 0x9FB00

[SECTION .text]
[BITS 16]
aps_start:
    mov     ax, 0   ;把段寄存器清零
    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     si, ax
    
enter_protected_mode:
    cli ;关中断

    mov ax, 0x9E00  ;因为地址超过16位了，因此ds不能为0了
    mov ds, ax
    lgdt [ds:0x00] ; smp中，bsp核和所有ap核共享一个gdt表 0x9E000

    in al, 0x92 ;开A20总线
    or al, 0b00000010
    out 0x92, al

    mov   eax, cr0 ; 设置保护模式
    or    eax , 1
    mov   cr0, eax

    mov ax, 0
    mov ds, ax
    jmp CODE_SECTOR:protected_mode

[BITS 32]
protected_mode:
    ; xchg bx, bx
    mov ax, DATA_SECTOR ;把所有的非代码段寄存器都设置为数据段选择子，实际上数据段和代码段重合的，因为gcc编译的c程序是基于平坦模式的。
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

ap_init_finish:
    mov eax, 1
spin_lock:
    xchg eax, [ap_boot_lock]  ;交换ax和ap_boot_lock的值，ax为1，如果ap_boot_lock为1则已经上锁，不改变值，如果ap_boot_lock为0，则获得锁
    test eax, eax
    jnz spin_pause

    inc dword [0x9F000]

spin_unlock:
    mov dword [ap_boot_lock], 0

    jmp $


spin_pause:
    pause   ;让cpu歇一会，减少功耗，降低流水线堵塞
    jmp spin_lock

