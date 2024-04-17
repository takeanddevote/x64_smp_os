

/* 
实现64位下的printk：
int printk(char *format, ...);

1、调用约定：x64下的类unix系统采用system V AMD64 ABI，使用6参数寄存器+栈传参：rdi、rsi、rdx、rcx、r8、r9，因此实现可变参数的关键点是
    先把寄存器入栈，再按照传统的栈传参的方式实现可变参数，因此需要使用汇编编写。

 */

int printk(char *format, ...)
{
    
}