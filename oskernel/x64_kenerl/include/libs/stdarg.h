#ifndef __STDARG_H__
#define __STDARG_H__

typedef char *va_list;

/* 
va_start：获取第一个参数的地址存到pArg
va_arg：通过参数地址pArg和该参数的类型type，解引用得到该参数值，并使pArg指向下一个参数
va_end：结束获取参数
 */

#define va_start(pArg, pFmt) (pArg = (va_list)(&pFmt) + sizeof(va_list)) //pArg指向第一个参数的地址
// pArg根据类型的大小type(当前参数的类型),指向下一个参数,然后再减去type又得到当前参数的地址,然后把当前参数的地址转类型并解引用返回
//对于栈传参，通过反汇编可以看到，每个参数都是以8字节大小传递的，即便参数可能不是8字节长度。
#define va_arg(pArg, type)  *((type*)((pArg += sizeof(va_list)) - sizeof(va_list)))
#define va_end(pArg) (pArg = 0)


#endif