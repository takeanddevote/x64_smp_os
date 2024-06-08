/* syscall执行流程

# 判断当前是否是长模式，这里的CS是指cs所关联的段描述符
IF (CS.L ≠ 1 ) or (IA32_EFER.LMA ≠ 1) or (IA32_EFER.SCE ≠ 1)
    (* Not in 64-Bit Mode or SYSCALL/SYSRET not enabled in IA32_EFER *)
    THEN #UD;
FI;

# rcx保存返回地址、r11保存rflags、rflags置位IA32_FMASK预设值、在RIP设置为IA32_LSTAR寄存器的地址，即中断函数地址、
RCX := RIP; (* Will contain address of next instruction *)
RIP := IA32_LSTAR;
R11 := RFLAGS;
RFLAGS := RFLAGS AND NOT(IA32_FMASK);

CS.Selector := IA32_STAR[47:32] AND FFFCH (* Operating system provides CS; RPL forced to 0 *)
(* Set rest of CS to a fixed value *)
CS.Base := 0; (* Flat segment *)
CS.Limit := FFFFFH; (* With 4-KByte granularity, implies a 4-GByte limit *)
CS.Type := 11; (* Execute/read code, accessed *) 
CS.S := 1;
CS.DPL := 0;
CS.P := 1;
CS.L := 1; (* Entry is to 64-bit mode *)
CS.D := 0; (* Required if CS.L = 1 *)
CS.G := 1; (* 4-KByte granularity *)
IF ShadowStackEnabled(CPL)
    THEN (* adjust so bits 63:N get the value of bit N–1, where N is the CPU’s maximum linear-address width *)
    IA32_PL3_SSP := LA_adjust(SSP);
    (* With shadow stacks enabled the system call is supported from Ring 3 to Ring 0 *)
    (* OS supporting Ring 0 to Ring 0 system calls or Ring 1/2 to ring 0 system call *)
    (* Must preserve the contents of IA32_PL3_SSP to avoid losing ring 3 state *)
FI;
CPL := 0;
IF ShadowStackEnabled(CPL)
    SSP := 0;
FI;
IF EndbranchEnabled(CPL)
    IA32_S_CET.TRACKER = WAIT_FOR_ENDBRANCH
    IA32_S_CET.SUPPRESS = 0
FI;
SS.Selector := IA32_STAR[47:32] + 8; (* SS just above CS *)
(* Set rest of SS to a fixed value *)
SS.Base := 0; (* Flat segment *)
SS.Limit := FFFFFH; (* With 4-KByte granularity, implies a 4-GByte limit *)
SS.Type := 3; (* Read/write data, accessed *)
SS.S := 1;
SS.DPL := 0;
SS.P := 1;
SS.B := 1; (* 32-bit stack segment *)
SS.G := 1; (* 4-KByte granularity *)

*/

#include "linux/syscall.h"
#include "linux/cpu.h"
#include "logger.h"

u32 SYSCALL_SELECTOR = (3 << 3) | 0b00;
u32 SYSRET_SELECTOR = (4 << 3) | 0b011;

extern void syscall_entry();
int syscall_init()
{
    /* 
        1、置位IA32_EFER.SCE，使能syscall
        2、IA32_LSTAR设置syscall调用的函数
        3、IA32_STAR设置syscall的cs和ss（64位内核态）、sysret的cs和ss（64位用户态）
        4、IA32_FMASK设置syscall时，需要清除rflags的哪些标志位（和中断cpu自动清除IF位不同，需要os指定），比如中断IF。
     */

    uint64_t val = read_msr(IA32_MSR_EFER);
    val = val | 0x01; //置位SCE
    write_msr(IA32_MSR_EFER, val);

    write_msr(IA32_MSR_LSTAR, (uint64_t)syscall_entry);

    /* 这里要加类型强转，不然结果不对，查看反汇编左移指令使用了32位寄存器，强转类型后才是64位寄存器 */
    val = ((uint64_t)SYSRET_SELECTOR << 48) | ((uint64_t)SYSCALL_SELECTOR << 32); //syscall/sysexit以cs的下一个index作为ss。
    // debug("IA32_MSR_STAR %x %x.\n", val >> 32, val);
    write_msr(IA32_MSR_STAR, val);

    val = 0x00047700; // 要求TF=IF=DF=AC=0；IOPL=00
    write_msr(IA32_MSR_FMASK, val);
}