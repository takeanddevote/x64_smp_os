BUILD_DIR := ./build
HD_IMG_NAME := "hd.img"

CFLAGS += -m32 #编译成32位的x86程序
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -nostdinc		# 不需要标准头文件
CFLAGS+= -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib		# 不需要标准库
CFLAGS+= -fno-stack-protector	# 不需要栈保护
CFLAGS:=$(strip ${CFLAGS})

inc=-I./oskernel/include/

#kernel目标文件
kernel_obj = $(BUILD_DIR)/init/main.o $(BUILD_DIR)/kernel/io.o $(BUILD_DIR)/kernel/head.o $(BUILD_DIR)/kernel/console.o \
	$(BUILD_DIR)/kernel/string.o $(BUILD_DIR)/kernel/printk.o $(BUILD_DIR)/kernel/vsprintf.o $(BUILD_DIR)/kernel/gdt.o	\
	$(BUILD_DIR)/kernel/idt.o $(BUILD_DIR)/kernel/int_isr.o $(BUILD_DIR)/kernel/keyboard.o

all: $(BUILD_DIR)/boot/boot.o $(BUILD_DIR)/boot/setup.o ${BUILD_DIR}/init/kernel.bin
	$(shell rm -rf $(HD_IMG_NAME))
	bximage	-q -hd=16 -func=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	@dd if=$(BUILD_DIR)/boot/boot.o of=hd.img bs=512 seek=0 count=1 conv=notrunc > /dev/null 2>&1
	@dd if=$(BUILD_DIR)/boot/setup.o of=hd.img bs=512 seek=1 count=2 conv=notrunc > /dev/null 2>&1
	@dd if=${BUILD_DIR}/init/kernel.bin of=hd.img bs=512 seek=3 count=50 conv=notrunc > /dev/null 2>&1
	@echo "\nbuild "${HD_IMG_NAME}" success......"

# 从elf文件取出纯机器码、提取符号生成map文件
${BUILD_DIR}/init/kernel.bin: ${BUILD_DIR}/init/kernel.elf
	objcopy -O binary ${BUILD_DIR}/init/kernel.elf ${BUILD_DIR}/init/kernel.bin
	nm ${BUILD_DIR}/init/kernel.elf | sort > ${BUILD_DIR}/init/kernel.map

#-m elf_i386指定链接成32位的可执行文件 -m elf_x86_64指定链接成64位的可执行文件
${BUILD_DIR}/init/kernel.elf: ${kernel_obj}
	ld -m elf_i386 $^ -o $@ -T link.ld


# 编译源文件
#汇编文件编译成elf 32位的.o文件，用来和c编译成的elf 32位.o文件进行链接
$(BUILD_DIR)/kernel/%.o: oskernel/kernel/asm/%.asm
	$(shell mkdir -p $(BUILD_DIR)/kernel)
	nasm -f elf32 -g $< -o $@

#直接把汇编文件编译成纯机器码文件，可以直接运行
$(BUILD_DIR)/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p $(BUILD_DIR)/boot)
	nasm $< -o $@

#把C源文件编译成elf 32位的.o文件
$(BUILD_DIR)/init/%.o: oskernel/init/%.c
	$(shell mkdir -p $(BUILD_DIR)/init)
	gcc ${CFLAGS} ${inc} -c $< -o $@
$(BUILD_DIR)/kernel/%.o: oskernel/kernel/chr_drv/%.c
	$(shell mkdir -p $(BUILD_DIR)/kernel)
	gcc ${CFLAGS} ${inc} -c $< -o $@
$(BUILD_DIR)/kernel/%.o: oskernel/kernel/%.c
	gcc ${CFLAGS} ${inc} -c $< -o $@
$(BUILD_DIR)/kernel/%.o: oskernel/lib/%.c
	$(shell mkdir -p $(BUILD_DIR)/lib)
	gcc ${CFLAGS} ${inc} -c $< -o $@

clean:
	$(shell rm -r $(BUILD_DIR))
	$(shell rm $(HD_IMG_NAME))

bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -hda hd.img