[SECTION .data]
; 定义局部全局变量
global g_printBuffer
g_argsNum: times 4 db 0x00
g_printBuffer: times 1024 db 0x00
g_args1: times 8 db 0x00
g_args2: times 8 db 0x00
g_args3: times 8 db 0x00
g_args4: times 8 db 0x00
g_args5: times 8 db 0x00
g_args6: times 8 db 0x00
retAddress: times 8 db 0x00



[SECTION .text]
[BITS 64]


;  1、怎么知道有多少个参数？因为6以后的参数是先push入栈的，然后call还入栈了返回地址，即进入函数后，rsp指向返回地址rip，
;  因此我们需要知道push入栈的参数个数，并把这些参数复制到连续的构造栈中。
; 自右向左，前6个参数是分别通过r9、r8 、rcx 、rdx、rsi、rdi
extern va_num
extern vsprintf
extern data_out
global printk
printk:
    ;实现思路：把返回地址保存下来，然后push6个参数入栈，形成连续的栈传参，然后ret前，再还原原来的栈，
    ; 思路和原来的一样的（原来的办法是挪动栈传递的参数部分，还需要判断传了多少个参数，现在只需挪动返回地址），但工作量更小，效率更高
    mov rax, [rsp]
    add rsp, 8
    push r9 
    push r8
    push rcx 
    push rdx
    push rsi
    push rdi
    push rax ; 返回地址

    mov rdx, rsp            ;args: printk的第二个形参的地址
    add rdx, 16
    mov rsi, [rsp+8]        ;fmt: 格式化字符串地址
    mov rdi, g_printBuffer  ;buf：
    call vsprintf

    mov rsi, rax
    mov rdi, g_printBuffer
    call data_out

    mov rax, [rsp] ; 栈平衡，并还原返回地址
    add rsp, 8*6
    mov [rsp], rax
    ret
    

;     ; 获取格式化字符串描述的参数格式+1=总传参个数
;     push rdi
;     call va_num
;     pop rdi
;     add eax, 1
;     call save_six_args

;     cmp eax, 6
;     ja .args_excel_six

;     jmp .args_not_excel_six

; .args_excel_six:    ; 传参超过6个参数，因此需要重新压栈超出的参数
;     sub eax, 6 ; 栈传参的个数

;     mov rcx, rax           ; 设置循环计数器为 n
;     lea rsi, [rsp+rax*8]   ;指向最后一个参数

; .loop_start:
;     mov rax, [rsi]    ; 读取rsi指向的参数值到rax中
;     push rax          ; 参数入栈
;     sub rsi, 8        ; 满减栈，地址递减8才符合自右向左传参
;     loop .loop_start  ; 重复循环直到 RCX 为 0

; .args_not_excel_six: ; 压栈6个寄存器参数
;     call restore_six_args
;     push r9 
;     push r8
;     push rcx 
;     push rdx
;     push rsi
;     push rdi

;     mov rdx, rsp            
;     add rdx, 8              ;args: printk的第二个形参的地址
;     mov rsi, rdi            ;fmt: 此时的rdi为传入printk的fmt指针
;     mov rdi, g_printBuffer  ;buf：
;     call vsprintf

; .outStr:
;     mov rsi, rax
;     mov rdi, g_printBuffer
;     call console_write

;     xor rax, rax    ;平栈
;     mov eax, [g_argsNum]
;     lea rax, [rax*8]
;     add rsp, rax
;     ret

; save_six_args:
;     mov [g_argsNum], eax ;保存参数个数到全局变量中
;     mov [g_args6], r9
;     mov [g_args5], r8
;     mov [g_args4], rcx
;     mov [g_args3], rdx
;     mov [g_args2], rsi
;     mov [g_args1], rdi
;     ret

; restore_six_args:
;     mov eax, [g_argsNum] ;保存参数个数到全局变量中
;     mov r9,  [g_args6], 
;     mov r8,  [g_args5], 
;     mov rcx, [g_args4],
;     mov rdx, [g_args3], 
;     mov rsi, [g_args2], 
;     mov rdi, [g_args1], 
;     ret