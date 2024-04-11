PHONY := all
all:

srctree := $(shell pwd)


include config.mk
include $(srctree)/scripts/Kbuild.include

# 顶层子目录
obj-y += oskernel/


PHONY += $(KBUILD_SRC)
$(KBUILD_SRC):
	$(call ensure_dir_exist,$@)

# 目标规则
all: scripts/link-firmware.sh $(KBUILD_SRC) boot loader
	$(Q) ([ -e $(KBUILD_SRC)/$(HD_IMG_NAME) ] && rm -f $(KBUILD_SRC)/$(HD_IMG_NAME); true)
	$(Q) $(MAKE) $(build) ./
	$(Q) $(LD) $(LDFLAGS) -o $(KBUILD_SRC)/$(TARGET) $(KBUILD_SRC)/built-in.o -T $(KBUILD_LDS)
	$(Q) echo "LD 	 built-in.o    "$(TARGET)

	$(Q) $(OBJCOPY) -O binary ${KBUILD_SRC}/$(TARGET) ${KBUILD_SRC}/$(TARGET).bin
	$(Q) $(NM) ${KBUILD_SRC}/$(TARGET) | sort > ${KBUILD_SRC}/$(TARGET).map

	$(Q) bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(KBUILD_SRC)/$(HD_IMG_NAME)
	$(Q) dd if=$(KBUILD_SRC)/oskernel/boot/built-in.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=$(KBUILD_SRC)/oskernel/loader/built-in.o of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=${KBUILD_SRC}/$(TARGET).bin of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=3 count=50 conv=notrunc > /dev/null 2>&1
	$(Q) dd if=${KBUILD_SRC}/$(TARGET)_x64.bin of=$(KBUILD_SRC)/$(HD_IMG_NAME) bs=512 seek=54 count=200 conv=notrunc > /dev/null 2>&1
	$(Q) echo "\nbuild "${HD_IMG_NAME}" success......"

PHONY += boot
PHONY += loader
boot:
	$(Q) $(MAKE) $(build) oskernel/boot

loader:
	$(Q) $(MAKE) $(build) oskernel/loader

ifeq ($(ARCH),X64)
all: x64_kernel
PHONY += x64_kernel
x64_kernel: scripts/link-firmware.sh
	$(Q) $(MAKE) -C $(srctree)/oskernel/x64_kenerl/ -f config.x64.mk
endif


PHONY += qemu
qemu: all
ifeq ($(ARCH),X86)
	$(Q) qemu-system-i386 -m 32M -boot c -hda $(KBUILD_SRC)$(HD_IMG_NAME)
else ifeq ($(ARCH),X64)
	$(Q) qemu-system-x86_64 \
    	-m 32M \
    	-boot c \
		-cpu Nehalem	\
    	-hda $(KBUILD_SRC)$(HD_IMG_NAME) 
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
	$(Q) qemu-system-x86_64 \
    	-m 32M \
    	-boot c \
		-cpu Nehalem	\
    	-hda $(KBUILD_SRC)$(HD_IMG_NAME) \
    	-s -S 
endif

PHONY += test
test: $(KBUILD_SRC)
	@ echo "test"

bochs: all $(if $(filter X64,$(ARCH)),x64_kernel,)
	bochs -q -f bochsrc

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