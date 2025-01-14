[ORG  0x7c00] ;运行地址

[SECTION .data]
second_part_addr equ 0x500


[SECTION .text]
[BITS 16]
global _start ;全局声明程序入口
_start:
    ; 设置屏幕模式为文本模式，清除屏幕
    ; mov ax, 3
    ; int 0x10

    ; xchg bx, bx ;bochs 断点
    ; xchg bx, bx

    mov     ax, 0  ;清零寄存器
    mov     ss, ax  
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     si, ax

    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x02
    mov al, 'h'
    int 0x10

    mov     si, msg
    call    print_str

    ; mov edi, second_part_addr
    ; mov ecx, 1
    ; mov bh, 2
    ; call hardDiskRead

    mov edi, second_part_addr ; 读到哪里
    mov ecx, 1      ; 从哪个扇区开始读
    mov bl, 2       ; 读多少扇区
    call read_hd

    ; xchg bx, bx
    ; xchg bx, bx

    jmp second_part_addr
    jmp $



    ; ;读写软盘 读到0x500
    ; xor ax, ax
    ; mov ds, ax

    ; mov     ch, 0   ; 0 柱面
    ; mov     dh, 0   ; 0 磁头
    ; mov     cl, 2   ; 2 扇区
    ; mov     bx, second_part_addr  ; 数据往哪读
    ; mov     ah, 0x02    ; 读盘操作
    ; mov     al, 1       ; 连续读几个扇区
    ; mov     dl, 0       ; 驱动器编号
    ; int     0x13

    ; jc  print_disk_error ;CF置位，出错
    ; mov si, print_disk_success
    ; call print_str

    ; mov si, 0x500
    ; mov cl, 0xa
    ; call print_hex

    ; jmp second_part_addr
    ; jmp $


print_disk_error:
    mov si, print_disk_err
    call print_str
    jmp $


;读取硬盘 edi 内存地址 ecx 起始扇区 bh 扇区数
hardDiskRead:
    mov dx, 0x1f2 ;设置扇区数
    mov al, bh
    out dx, al

    mov dx, 0x1f3 ;设置LBA 低八位
    mov al, cl
    out dx, al

    mov dx, 0x1f4 ;设置LBA 中八位
    mov al, ch
    out dx, al

    mov dx, 0x1f5 ;设置LBA 高八位
    shr ecx, 0x10 ;右移16位获取高16位
    mov al, cl
    out dx, al

    mov dx, 0x1f6 ;设置LBA最高四位、设置LBA寻址、主盘从盘
    mov al, 0xe0
    and ch, 0x0f ;ch高四位清零
    or al, ch ;复制低四位到al中作为LBA的最高四位
    out dx, al

    mov dx, 0x1f7 ;命令端口写入0x20表示开始读硬盘
    mov al, 0x20
    out dx, al

    xor cx, cx
    mov cl, bh ;循环次数，一个扇区循环一次
.start_read:
    push cx
    call waitHdReady
    call readHdOneSector
    pop cx
    loop .start_read ;运行到loop时cx先减去1，再判断是否等于0，是则执行后面的代码，否则循环
    ret


waitHdReady:
    mov dx, 0x1f7
.check:
    in ax, dx ;读取0x1f7端口的值，b0=0（写入的指令0x20无错误）、b3=1（控制器数据准备好了）、b7=0（控制器不忙执行指令）
    and ax, 0b1000_1001
    cmp ax, 0b0000_1000
    jne .check
    ret


readHdOneSector:
    mov dx, 0x1f0
    mov cx, 256 ;一次读一个word，循环256次即一个扇区
.readWord:
    in ax, dx
    mov [edi], ax ;2字节的数据写入内存
    add edi, 2 ;内存地址+2
    loop .readWord
    ret

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

msg:
    db "hello, world", 10, 13, 0
print_disk_err:
    db "read disk error", 10, 13, 0

print_disk_success:
    db "read disk success", 10, 13, 0


;16进制打印一段内存。si指向内存地址；cl要打印的字节数
print_hex:
    xor bx, bx
    mov ch, [si] ;读取内存的一个字节到ah
    mov bl, ch
    shr bl, 4
    mov al, [.hexChars+bx]
    call .print_al

    xor bx, bx
    mov bl, ch
    and bl, 0x0f
    mov al, [.hexChars+bx]
    call .print_al

    inc si
    dec cl
    cmp cl, 0x00
    jne print_hex

    mov al, 10
    call .print_al
    mov al, 13
    call .print_al
    ret

.print_al:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
    int 0x10
    ret
.hexChars: 
    db "0123456789ABCDEF"



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













times 510 - ($ - $$) db 0
db 0x55, 0xaa