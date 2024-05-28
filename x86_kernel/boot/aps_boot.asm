[ORG 0x9C000]

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
    
    mov eax, 1
spin_lock:
    mov ax, aps_start   ;获取链接地址，因为链接地址超过64kb了，在ds设置为0下，不能访问超过64KB了，因此需要修改ds。
    shr ax, 4
    mov ds, ax

    xchg eax, [ap_boot_lock]  ;交换ax和ap_boot_lock的值，ax为1，如果ap_boot_lock为1则已经上锁，不改变值，如果ap_boot_lock为0，则获得锁
    test eax, eax
    jnz spin_pause


    mov ax, 0x9FB0
    mov ds, ax
    inc dword [ds:0x00]

spin_unlock:

    mov ax, aps_start
    shr ax, 4
    mov ds, ax
    mov dword [ap_boot_lock], 0

    jmp $


spin_pause:
    pause   ;让cpu歇一会，减少功耗，降低流水线堵塞
    jmp spin_lock

