[SECTION .data]
printfBuffer: times 4096 db 0x00


[SECTION .text]
[BITS 64]
global printf
extern write
extern vsprintf
printf:
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
    mov rdi, printfBuffer  ;buf：
    call vsprintf

    mov rdx, rax
    mov rsi, printfBuffer
    mov rdi, 0 ; 标准输出
    call write

    mov rax, [rsp] ; 栈平衡，并还原返回地址
    add rsp, 8*6
    mov [rsp], rax
    ret