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

all: $(BUILD_DIR)/boot/boot.o $(BUILD_DIR)/boot/setup.o ${BUILD_DIR}/init/kernel.bin
	$(shell rm -rf $(HD_IMG_NAME))
	bximage	-q -hd=16 -func=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=$(BUILD_DIR)/boot/boot.o of=hd.img bs=512 seek=0 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/boot/setup.o of=hd.img bs=512 seek=1 count=2 conv=notrunc
	dd if=${BUILD_DIR}/init/kernel.bin of=hd.img bs=512 seek=3 count=50 conv=notrunc

${BUILD_DIR}/init/kernel.bin: ${BUILD_DIR}/init/kernel.elf
	objcopy -O binary ${BUILD_DIR}/init/kernel.elf ${BUILD_DIR}/init/kernel.bin
	nm ${BUILD_DIR}/init/kernel.elf | sort > ${BUILD_DIR}/init/kernel.map

#-m elf_i386指定链接成32位的可执行文件 -m elf_x86_64指定链接成64位的可执行文件
${BUILD_DIR}/init/kernel.elf:  $(BUILD_DIR)/init/head.o $(BUILD_DIR)/init/main.o 
	ld -m elf_i386 $^ -o $@ -T link.ld

$(BUILD_DIR)/boot/head.o: oskernel/boot/head.asm
	nasm -f elf32 -g $< -o $@

$(BUILD_DIR)/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p $(BUILD_DIR)/boot)
	nasm $< -o $@

$(BUILD_DIR)/init/%.o: oskernel/init/%.c
	$(shell mkdir -p $(BUILD_DIR)/init)
	gcc ${CFLAGS} -c $< -o $@


clean:
	$(shell rm -r $(BUILD_DIR))
	$(shell rm $(HD_IMG_NAME))

bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -hda hd.img