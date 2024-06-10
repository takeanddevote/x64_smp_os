
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
TARGET ?= netstack.elf

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
LINUXINCLUDRE := 
KBUILD_LDS := 

# 配置编译参数
CFLAGS := $(DEBUG)
	
CFLAGS :=$(strip ${CFLAGS})

LDFLAGS := 

ASFLAGS :=
LIBPATH := -L$(shell pwd)/netstack/libs/lib -Wl,-rpath=$(shell pwd)/netstack/libs/lib
LIBS := -lpcap 

export CC CPP CXX AS LD NM AR STRIP OBJCOPY OBJDUMP
export CFLAGS LDFLAGS ASFLAGS LIBPATH LIBS
export MAKE Q srctree KBUILD_SRC KBUILD_LDS TARGET

obj-y += netstack/