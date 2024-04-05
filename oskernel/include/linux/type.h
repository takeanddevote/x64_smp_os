#ifndef __TYPE_H__
#define __TYPE_H__

typedef unsigned char u8;
typedef char s8;

typedef unsigned short u16;
typedef short s16;

typedef unsigned int u32;
typedef int s32;
typedef unsigned int size_t;
typedef signed int ssize_t;

typedef unsigned long long u64;
typedef signed long long s64;

typedef char bool;
#define true    1
#define false   0


#define EOF -1 // END OF FILE
#define NULL ((void *)0) // 空指针
#define EOS '\0' // 字符串结尾

#endif /* __TYPE_H__ */