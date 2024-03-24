PHONY := all
all:

# 1、把汇编文件编译成二进制机器码文件
# 2、把汇编文件编译成32位的elf可链接目标文件
# 3、把C文件编译成32位的elf可链接目标文件
# 4、把所有的可链接目标文件链接成可执行文件
# 5、使用objcopy将可执行文件制作为二进制机器码文件


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
LINUXINCLUDRE := -I$(srctree)/oskernel/include
KBUILD_LDS := ./vmlinux.lds

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
CFLAGS :=$(strip ${CFLAGS})
LDFLAGS := -m elf_i386
ASFLAGS := -f elf32 $(DEBUG)

export CC CPP CXX AS LD NM AR STRIP OBJCOPY OBJDUMP
export CFLAGS LDFLAGS ASFLAGS
export MAKE Q srctree KBUILD_SRC KBUILD_LDS

# 顶层子目录
obj-y += oskernel/

PHONY += chkKbuildSrc
chkKbuildSrc:
ifeq ($(wildcard $(KBUILD_SRC)),)
	$(Q) mkdir -p $(KBUILD_SRC)
endif

# 目标规则
all: chkKbuildSrc boot loader
	$(Q) ([ -e $(KBUILD_SRC)/$(HD_IMG_NAME) ] && rm -f $(KBUILD_SRC)/$(HD_IMG_NAME); true)
	$(Q) $(MAKE) -C ./ -f $(srctree)/scripts/Makefile.build
	$(Q) $(LD) $(LDFLAGS) -o $(KBUILD_SRC)/$(TARGET) $(KBUILD_SRC)/built-in.o -T $(KBUILD_LDS)
	$(Q) echo "LD 	 built-in.o    "$(TARGET)

	$(Q) $(OBJCOPY) -O binary ${KBUILD_SRC}/$(TARGET) ${KBUILD_SRC}/$(TARGET).bin
	$(Q) $(NM) ${KBUILD_SRC}/$(TARGET) | sort > ${KBUILD_SRC}/$(TARGET).map

	$(Q) bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(KBUILD_SRC)/$(HD_IMG_NAME)
	$(Q) dd if=$(KBUILD_SRC)/oskernel/boot/built-in.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=$(KBUILD_SRC)/oskernel/loader/built-in.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=${KBUILD_SRC}/$(TARGET).bin of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=3 count=50 conv=notrunc > /dev/null 2>&1
	$(Q) echo "\nbuild "${HD_IMG_NAME}" success......"

PHONY += boot
PHONY += loader
boot:
	$(Q) $(MAKE) -C oskernel/boot -f $(srctree)/scripts/Makefile.build

loader:
	$(Q) $(MAKE) -C oskernel/loader -f $(srctree)/scripts/Makefile.build

qemu: all
	$(Q) qemu-system-x86_64 -m 32M -boot c -cpu Opteron_G5 -hda $(KBUILD_SRC)$(HD_IMG_NAME)



PHONY += clean
PHONY += distclean
PHONY += mrproper
clean:
	$(Q) -rm -f $(shell find -name "*.o")
	$(Q) -rm -f $(KBUILD_SRC)/$(TARGET)
	$(Q) true

distclean:
	$(Q) - rm -f $(shell [ -e $(KBUILD_SRC) ] && find $(KBUILD_SRC) -name "*.o")
	$(Q) - rm -f $(shell [ -e $(KBUILD_SRC) ] && find $(KBUILD_SRC) -name "*.d")
	$(Q) - rm -f $(KBUILD_SRC)/$(TARGET)
	$(Q) true

mrproper : distclean
	$(Q) -rm -rf $(KBUILD_SRC)
	$(Q) true

.PHONY : $(PHONY)