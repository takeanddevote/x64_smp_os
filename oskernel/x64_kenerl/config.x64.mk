
PHONY += _bulid
_bulid:

include $(srctree)/scripts/Kbuild.include

# 配置中间文件输出目录
KBUILD_SRC := $(srctree)/.build/

# 配置可执行程序名字
TARGET := vmlinux_x64

# 配置编译参数
CFLAGS := $(DEBUG) 
CFLAGS += $(LINUXINCLUDRE)
CFLAGS += -m64 #编译成32位的x86程序
CFLAGS += -masm=intel
CFLAGS += -fno-builtin	# 不需要 gcc 内置函数
CFLAGS += -nostdinc		# 不需要标准头文件
CFLAGS += -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS += -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS += -nostdlib		# 不需要标准库
CFLAGS += -fno-stack-protector	# 不需要栈保护
CFLAGS :=$(strip ${CFLAGS})
LDFLAGS := -m elf_x86_64
ASFLAGS := -f elf64 $(DEBUG)

export CC CPP CXX AS LD NM AR STRIP OBJCOPY OBJDUMP
export CFLAGS LDFLAGS ASFLAGS
export MAKE Q srctree KBUILD_SRC KBUILD_LDS

_bulid:
	$(Q) $(MAKE) $(build) ./
	$(Q) $(LD) $(LDFLAGS) -o $(KBUILD_SRC)/$(TARGET) $(KBUILD_SRC)/oskernel/x64_kenerl/built-in.o -T $(srctree)/vmlinux_x64.lds
	$(Q) echo "LD 	 built-in.o    "$(TARGET)

	$(Q) $(OBJCOPY) -O binary $(KBUILD_SRC)/$(TARGET) $(KBUILD_SRC)/$(TARGET).bin
	$(Q) $(NM) $(KBUILD_SRC)/$(TARGET) | sort > $(KBUILD_SRC)/$(TARGET).map

.PHONY : $(PHONY)