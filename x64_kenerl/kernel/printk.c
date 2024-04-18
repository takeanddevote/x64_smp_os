

/* 
实现64位下的printk：
int printk(char *format, ...);

1、调用约定：x64下的类unix系统采用system V AMD64 ABI，使用6参数寄存器+栈传参，前6个参数自右向左(4字节整数)：r9d、r8d 、ecx 、edx、esi、edi，
    因此实现可变参数的关键点是先把寄存器入栈，再按照传统的栈传参的方式实现可变参数。
2、调用printk时，编译器通过printk声明来识别可变参数，然后自动按照system V AMD64 ABI约定传参，并且在函数内自动生成如下代码。

  4004d6:	55                   	push   rbp  # 保存上个函数的rbp栈帧基址
  4004d7:	48 89 e5             	mov    rbp,rsp  # 设置当前函数的栈帧起始地址
  4004da:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi  #先把前6个寄存器传参入栈，
  4004dd:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
  4004e0:	89 55 f4             	mov    DWORD PTR [rbp-0xc],edx
  4004e3:	89 4d f0             	mov    DWORD PTR [rbp-0x10],ecx
  4004e6:	44 89 45 ec          	mov    DWORD PTR [rbp-0x14],r8d
  4004ea:	44 89 4d e8          	mov    DWORD PTR [rbp-0x18],r9d

    这里涉及到两个问题：
        1、6以后的参数是自右向左push入栈的，其余参数直接设置进寄存器，然后再call调用函数，这时会push一个返回地址入栈，这就
        导致前6和后续参数中间断开了，而我们的va_list是基于参数连续地址的（可见实际的va_list应该很复杂）.如果我们这样
            int printk(char *format, ...) {
                va_list args;
                va_start(args, format);
            }
        此时的format定义在栈的[rbp-0x4]这个位置。

        2、编译器并不一定按照自右向左的方式把寄存器参数存入栈中。因此，我们需要使用汇编来构造符合栈传参规范的汇编，以免被编译器自动生成的代码影响。
        3、自己构造栈传参时，怎么知道有多少个参数呢？以及参数的长度，因此参数长度不同，会使用64位寄存器的不同位数。

  d3:   6a f8                   push   0xfffffffffffffff8
  d5:   6a f7                   push   0xfffffffffffffff7
  d7:   49 c7 c1 f6 ff ff ff    mov    r9,0xfffffffffffffff6
  de:   49 c7 c0 f5 ff ff ff    mov    r8,0xfffffffffffffff5
  e5:   48 c7 c1 f4 ff ff ff    mov    rcx,0xfffffffffffffff4
  ec:   48 c7 c2 f3 ff ff ff    mov    rdx,0xfffffffffffffff3
  f3:   48 c7 c6 f2 ff ff ff    mov    rsi,0xfffffffffffffff2
  fa:   48 c7 c7 f1 ff ff ff    mov    rdi,0xfffffffffffffff1
 101:   e8 00 00 00 00          call   106 <x64_kernel_main+0x69>
 */

/* 
1、对于超出6个的其余参数是自右向左push入栈的，因此我们需要把前6个寄存器参数按照传参自右向左顺序push入栈，即可实现纯栈传参。
2、这是编译器自动生成的8参数（不是使用... 是8个形参）传递处理汇编：
  4004d6:	55                   	push   rbp
  4004d7:	48 89 e5             	mov    rbp,rsp
  4004da:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi  #先把前6个寄存器传参入栈，
  4004dd:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
  4004e0:	89 55 f4             	mov    DWORD PTR [rbp-0xc],edx
  4004e3:	89 4d f0             	mov    DWORD PTR [rbp-0x10],ecx
  4004e6:	44 89 45 ec          	mov    DWORD PTR [rbp-0x14],r8d
  4004ea:	44 89 4d e8          	mov    DWORD PTR [rbp-0x18],r9d

  4004ee:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [rbp-0x4],0x1  #对形参赋值
  4004f5:	c7 45 f8 02 00 00 00 	mov    DWORD PTR [rbp-0x8],0x2
  4004fc:	c7 45 f4 03 00 00 00 	mov    DWORD PTR [rbp-0xc],0x3
  400503:	c7 45 f0 04 00 00 00 	mov    DWORD PTR [rbp-0x10],0x4
  40050a:	c7 45 ec 05 00 00 00 	mov    DWORD PTR [rbp-0x14],0x5
  400511:	c7 45 e8 06 00 00 00 	mov    DWORD PTR [rbp-0x18],0x6
  400518:	c7 45 10 07 00 00 00 	mov    DWORD PTR [rbp+0x10],0x7
  40051f:	c7 45 18 08 00 00 00 	mov    DWORD PTR [rbp+0x18],0x8
*/



int va_num(char *format)
{
    char *p = format;
    int num = 0;
    while(*p != '\0') {
        if(*p == '%')
            num++;
        p++;
    }
    return num;
}