BUILD_DIR := ./build
HD_IMG_NAME := "hd.img"

all: $(BUILD_DIR)/boot/boot.o $(BUILD_DIR)/boot/setup.o
	$(shell rm -rf $(HD_IMG_NAME))
	bximage	-q -hd=16 -func=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=$(BUILD_DIR)/boot/boot.o of=hd.img bs=512 seek=0 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/boot/setup.o of=hd.img bs=512 seek=1 count=1 conv=notrunc

$(BUILD_DIR)/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p $(BUILD_DIR)/boot)
	nasm $< -o $@

clean:
	$(shell rm -r $(BUILD_DIR))
	$(shell rm $(HD_IMG_NAME))

bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -fda hd.img