#include "libs/unistd.h"

/* 
	调用号：rax
	前6个函数参数：rdi、rsi 、rdx、r10、r8、r9
	后续参数：栈传参
	返回值：rax

    __asm__ volatile (
        "syscall"
        : "=a" (result)                 // 输出：系统调用返回值存储在rax中
        : "a" (number),                 // 输入：系统调用号存储在rax中
          "D" (arg1),                   // 输入：第一个参数存储在rdi中
          "S" (arg2),                   // 输入：第二个参数存储在rsi中
          "d" (arg3),                   // 输入：第三个参数存储在rdx中
          "r10" (arg4),                 // 输入：第四个参数存储在r10中
          "r8" (arg5),                  // 输入：第五个参数存储在r8中
          "r9" (arg6)                   // 输入：第六个参数存储在r9中
        : "rcx", "r11", "memory"        // 被破坏的寄存器列表：rcx和r11以及内存
    );

 */

int write(int fd, const void *buf, int count)
{
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(__NR_write), "d"(count), "S"(buf), "D"(fd)
        : "rcx", "r11", "memory"
    );
    return ret;
}