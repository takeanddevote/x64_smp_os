#ifndef __ASM_H__
#define __ASM_H__
#include "linux/type.h"

u8 inByte(u32 port);
void outByte(u32 port, u8 byte);
u16 inWord(u32 port);
void outWord(u32 port, u16 byte);


#endif /* __ASM_H__ */
