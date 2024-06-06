#ifndef __EXCEPTIONERRORCODE_H__
#define __EXCEPTIONERRORCODE_H__
#include "linux/type.h"

typedef struct {
    u16 EXT : 1;
    u16 IDT : 1;
    u16 TI : 1;
    u16 segmentSelectorIndex : 10;
    u16 reserved1 : 3;
    u16 reserved2;
} __attribute__((packed)) normal_errCode;

typedef struct {
    u16 P : 1;
    u16 WR : 1;
    u16 US : 1;
    u16 RSVD : 1;
    u16 ID : 1;
    u16 PK : 1;
    u16 SS : 1;
    u16 HLAT : 1;
    u16 reaserved0 : 7;
    u16 SGX : 1;
    u16 reserved1;
} __attribute__((packed)) PF_errCode;

typedef struct {
    u16 CPEC : 15;
    u16 ENCL : 1;
    u16 reserved;
} __attribute__((packed)) CP_errCode;

void handle_normal_errcode(u32 error_code, u64 *stack, u64 id);
void handle_PF_errcode(u32 error_code, u64 *stack, u64 id);
void handle_CP_errcode(u32 error_code, u64 *stack, u64 id);

#endif /* __EXCEPTIONERRORCODE_H__ */
