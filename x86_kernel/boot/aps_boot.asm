[ORG 0x8000]
;注意链接地址不要超过0xffff，不然跳转实模式下jmp时是长跳转，对跳转地址不太好处理，因为地址超过16位了。

[SECTION .gdt]
CODE_SECTOR equ (1 << 3)
DATA_SECTOR equ (2 << 3)


[SECTION .data]
ap_stack_lock: dd 0x0000   ;自旋锁变量 0x9FB00
ap_stack_offset: dd 0x9000   ;自旋锁变量 0x9FB00

ap_boot_lock equ 0xb000
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
    mov ax, DATA_SECTOR ;把所有的非代码段寄存器都设置为数据段选择子，实际上数据段和代码段重合的，因为gcc编译的c程序是基于平坦模式的。
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, 1
.spin_lock:
    xchg eax, [ap_stack_lock]  ;交换ax和ap_boot_lock的值，ax为1，如果ap_boot_lock为1则已经上锁，不改变值，如果ap_boot_lock为0，则获得锁
    test eax, eax
    jnz spin_pause


    mov eax, [ap_stack_offset]
    ; 初始化APs栈
    mov esp, eax
    mov ebp, esp

    add eax, 128
    mov [ap_stack_offset], eax

.spin_unlock:
    mov dword [ap_stack_lock], 0
    jmp .enter_64_long_mode

.spin_pause:
    pause   ;让cpu歇一会，减少功耗，降低流水线堵塞
    jmp .spin_lock

.enter_64_long_mode:
    ; 启动PAE
    mov eax, cr4
    bts eax, 5
    mov cr4, eax

    ; 加载PML4页表，使用bsp同一个页表，因为APs最终会和bsp运行一套代码。
    mov eax, 0x90000
    mov cr3, eax

    ; 长模式使能
    mov ecx, 0x0c0000080
    rdmsr
    bts eax, 8
    wrmsr

    ; 开启分页
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 和bsp使用同一个段选择子，跳转64位代码，进入64位模式
    ; jmp 0x18:long_mode
    push 0x18
    push long_mode   ; eip x64内核的start address

    xor eax, eax
    mov eax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    retf ; retf 长跳转返回，从栈中弹出：eip、cs

[BITS 64]
long_mode:
ap_init_finish:
    mov eax, 1
spin_lock:
    xchg eax, [ap_boot_lock]  ;交换ax和ap_boot_lock的值，ax为1，如果ap_boot_lock为1则已经上锁，不改变值，如果ap_boot_lock为0，则获得锁
    test eax, eax
    jnz spin_pause


    ; ; 初始化APs栈
    ; mov eax, [0x9F000]
    ; imul eax, eax, 128
    ; add eax, 0x9000

    ; mov esp, 0x9000
    ; mov ebp, esp

    inc dword [0x9F000]

spin_unlock:
    ; mov dword [ap_boot_lock], 0

    mov rax, [0xA000]   ;跳转bsp核的c入口函数
    jmp rax

.hlt:
    sti
    hlt
    jmp .hlt

spin_pause:
    pause   ;让cpu歇一会，减少功耗，降低流水线堵塞
    jmp spin_lock

