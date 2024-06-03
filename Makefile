PHONY := all
all:

srctree := $(shell pwd)

CONFIG_SHELL := $(shell if [ -x "$$SHELL" ]; then echo $$SHELL; \
		else if [ -x /bin/bash ]; then echo /bin/bash; \
		else echo sh; fi ; fi)

-include $(srctree)/.config


include config.mk
include $(srctree)/scripts/Kbuild.include

# 顶层子目录
obj-y += x86_kernel/
# header-y := configs   #生成的配置头文件目录

buildSrcRoot ?= ./
PHONY += buildSrc
buildSrc : $(cfg_dir) $(src_file)
	$(Q) $(MAKE) $(build_src) $(buildSrcRoot) clean-file=yes

buildIncRoot ?= ./
PHONY += buildInc
buildInc : $(cfg_dir) $(inc_file)
	$(Q) $(MAKE) $(build_inc) $(buildIncRoot) clean-file=yes
	
PHONY += buildcfgFile
buildcfgFile: $(cfg_dir) $(cfg_ini_out)
	$(Q) $(CONFIG_SHELL) $(srctree)/scripts/cfg-firmware.sh -h $(cfg_ini_out) $(cfg_header_file)
	$(Q) $(CONFIG_SHELL) $(srctree)/scripts/cfg-firmware.sh -m $(cfg_ini_out) $(cfg_mk_file)
	
forEnsureFilesExist := $(inc_file) $(cfg_mk_file) $(cfg_header_file) $(src_file)
PHONY += $(forEnsureFilesExist)
$(forEnsureFilesExist):
	$(call ensure_file_exist,$@)
	
forEnsureDirsExist := $(cfg_dir) $(KBUILD_SRC) $(PUBLIC_SRC)
PHONY += $(forEnsureDirsExist)
$(forEnsureDirsExist):
	$(call ensure_dir_exist,$@)

forCheckFilesExist := $(cfg_ini_out)
PHONY += $(forCheckFilesExist)
$(forCheckFilesExist):
	$(call check_file_exist,$@)

# 目标规则
all: scripts/link-firmware.sh $(KBUILD_SRC) boot loader
	$(Q) ([ -e $(KBUILD_SRC)/$(HD_IMG_NAME) ] && rm -f $(KBUILD_SRC)/$(HD_IMG_NAME); true)
	$(Q) $(MAKE) $(build) ./
	$(Q) $(LD) $(LDFLAGS) -o $(KBUILD_SRC)/$(TARGET) $(KBUILD_SRC)/built-in.o -T $(KBUILD_LDS)
	$(Q) echo "LD 	 built-in.o	"$(TARGET)

	$(Q) $(OBJCOPY) -O binary ${KBUILD_SRC}/$(TARGET) ${KBUILD_SRC}/$(TARGET).bin
	$(Q) $(NM) ${KBUILD_SRC}/$(TARGET) | sort > ${KBUILD_SRC}/$(TARGET).map

	$(Q) bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(KBUILD_SRC)/$(HD_IMG_NAME)
	$(Q) dd if=$(KBUILD_SRC)/x86_kernel/boot/boot.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=$(KBUILD_SRC)/x86_kernel/loader/built-in.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=${KBUILD_SRC}/$(TARGET).bin of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=3 count=50 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=${KBUILD_SRC}/$(TARGET)_x64.bin of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=54 count=200 conv=notrunc > /dev/null 2>&1

	$(Q) dd if=$(KBUILD_SRC)/x86_kernel/boot/aps_boot.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=255 count=1 conv=notrunc > /dev/null 2>&1
	$(Q) echo "\nbuild "${HD_IMG_NAME}" success......"

PHONY += boot
PHONY += loader
boot:
	$(Q) $(MAKE) $(build) x86_kernel/boot

loader:
	$(Q) $(MAKE) $(build) x86_kernel/loader

ifeq ($(ARCH),X64)
all: x64_kernel
PHONY += x64_kernel
x64_kernel: scripts/link-firmware.sh
	$(Q) $(MAKE) -C $(srctree)/x64_kenerl/ -f config.x64.mk
endif


PHONY += qemu
qemu: all
ifeq ($(ARCH),X86)
	$(Q) qemu-system-i386 -m 32M -boot c -hda $(KBUILD_SRC)$(HD_IMG_NAME)
else ifeq ($(ARCH),X64)
	$(Q) clear
	$(Q) ps -e | grep qemu | sed 's/^[ \t]*//' | cut -d' ' -f1 | xargs kill -9
	$(Q) qemu-system-x86_64 \
		-m 32M \
		-boot c \
		-cpu Nehalem -smp cores=1,threads=2	\
		-hda $(KBUILD_SRC)$(HD_IMG_NAME)	\
		-nographic \
		&

endif

PHONY += gdbqemu
gdbqemu: all
ifeq ($(ARCH),X86)
	$(Q) qemu-system-i386 \
		-m 32M \
		-boot c \
		-hda $(KBUILD_SRC)$(HD_IMG_NAME) \
		-s -S -nographic
else ifeq ($(ARCH),X64)
	$(Q) clear
	$(Q) ps -e | grep qemu | sed 's/^[ \t]*//' | cut -d' ' -f1 | xargs kill -9
	$(Q) qemu-system-x86_64 \
		-m 32M \
		-boot c \
		-cpu Nehalem -smp cores=1,threads=2	\
		-hda $(KBUILD_SRC)$(HD_IMG_NAME) \
		-nographic \
		-s -S 	\
		&
endif

bochs: all $(if $(filter X64,$(ARCH)),x64_kernel,)
	bochs -q -f bochsrc




PHONY += config old_config _config
_config: old_config_file := $(cfg_ini_old)
export old_config_file
_config: mrproper $(cfg_dir)
	$(Q) $(CONFIG_SHELL) $(srctree)/scripts/cfg-firmware.sh $(option) $(cfg_ini) $(cfg_ini_out)

config: option = -c
config: _config buildcfgFile

old_config: option = -o
old_config: _config buildcfgFile

PHONY += clean
PHONY += distclean
PHONY += mrproper

clean:
	$(Q) -rm -f $(shell find -name "*.o")
	$(Q) -rm -f $(KBUILD_SRC)/$(TARGET)
	$(Q) /bin/true

distclean:
	$(Q) -rm -f $(shell [ -e $(KBUILD_SRC) ] && find $(KBUILD_SRC) -name "*.o")
	$(Q) -rm -f $(shell [ -e $(KBUILD_SRC) ] && find $(KBUILD_SRC) -name "*.d")
	$(Q) -rm -f $(KBUILD_SRC)/$(TARGET)
	$(Q) /bin/true

mrproper:
	$(Q) -rm -rf $(KBUILD_SRC) $(cfg_dir) $(PUBLIC_SRC)
	$(Q) /bin/true
	
define help
	@echo "help"
endef

PHONY += help
help:
	$(Q) $(help)

PHONY += FORCE
.PHONY : $(PHONY)