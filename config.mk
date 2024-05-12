# 1、把汇编文件编译成二进制机器码文件
# 2、把汇编文件编译成32位的elf可链接目标文件
# 3、把C文件编译成32位的elf可链接目标文件
# 4、把所有的可链接目标文件链接成可执行文件
# 5、使用objcopy将可执行文件制作为二进制机器码文件

ARCH ?= X64

# 配置make参数
MQ	:= -s
MAKE := make $(MQ)

# 静默输出
Q	:= @

# 调试
DEBUG := -g

srctree := $(shell pwd)

# 配置中间文件输出目录
KBUILD_SRC := $(srctree)/.build/

# 配置可执行程序名字
TARGET := vmlinux

HD_IMG_NAME := "hd.img"

# 配置编译工具链
CROSS_COMPILE :=

CC 		:= $(CROSS_COMPILE)gcc
CPP		:= $(CC) -E
CXX		:= $(CROSS_COMPILE)g++
AS		:= $(CROSS_COMPILE)nasm
LD 		:= $(CROSS_COMPILE)ld
NM		:= $(CROSS_COMPILE)nm
AR		:= $(CROSS_COMPILE)ar
STRIP	:= $(CROSS_COMPILE)strip
OBJCOPY	:= $(CROSS_COMPILE)objcopy
OBJDUMP	:= $(CROSS_COMPILE)objdump

# 头文件路径
LINUXINCLUDRE := -I$(srctree)/x86_kernel/include
KBUILD_LDS := $(srctree)/vmlinux.lds

# 配置编译参数
CFLAGS := $(DEBUG) 
CFLAGS += $(LINUXINCLUDRE)
CFLAGS += -m32 #编译成32位的x86程序
CFLAGS += -masm=intel
CFLAGS += -fno-builtin	# 不需要 gcc 内置函数
CFLAGS += -nostdinc		# 不需要标准头文件
CFLAGS += -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS += -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS += -nostdlib		# 不需要标准库
CFLAGS += -fno-stack-protector	# 不需要栈保护
CFLAGS += -Wno-int-to-pointer-cast -Wno-int-conversion -Wno-pointer-to-int-cast
CFLAGS :=$(strip ${CFLAGS})
LDFLAGS := -m elf_i386
ASFLAGS := -f elf32 $(DEBUG)

export CC CPP CXX AS LD NM AR STRIP OBJCOPY OBJDUMP
export CFLAGS LDFLAGS ASFLAGS
export MAKE Q srctree KBUILD_SRC KBUILD_LDS