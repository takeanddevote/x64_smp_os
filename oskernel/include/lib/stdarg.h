#ifndef __STDARG_H__
#define __STDARG_H__

typedef char* va_list;

/* 
1、对于__cdecl调用约定来说：只使用栈传参，并且传参顺序时自右向左。
2、问题1：使用栈传参，形参的地址挨着的，那不同类型的形参占用多少字节呢？
在x86-64-linux下测试，char-4B、short-4B、int-8B、long-8B等等，可见形参占用空间是会变的，因此可变参数的左边的第一个参数的类型
一般是指针。
3、可变参数的工作原理：在__cdecl下利用形参的地址是挨着的，获取左边第一个参数的地址，然后跳转到可变参数的第一个参数地址（默认加指针的长度），
第一个参数的长度，可以根据格式化字符串来判断。根据格式化字符串中的%的个数即可递推所有参数以及参数的长度。
 */
#define va_start(p, count) (p = (va_list)&count + sizeof(va_list))
#define va_arg(p, t) (*(t*)(p += sizeof(va_list) - sizeof(va_list))) //p先指向下一个参数地址，然后再返回当前参数地址，再转为传入的t*指针类型，再取值
#define va_end(p) (p = 0)

#endif
