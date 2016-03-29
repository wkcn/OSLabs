BUILD_DIR = build
IMAGE_NAME=disk.img
CC = g++
CFLAGS = -Iinclude -c -m16 -march=i386 -masm=intel -nostdlib -ffreestanding -mpreferred-stack-boundary=2 -lgcc -shared
OBJCOPY=objcopy

AS = nasm
ASFLAGS =
LD = ld
LDFLAGS = -m elf_i386 -N

QEMU = qemu-system-i386
QEMUFLAGS = -fda

BOCHS = bochs
BOCHSFLAGS = -q -f bochsrc.bxrc

all: clean build_dir write_image

build_dir:
	-mkdir $(BUILD_DIR)

write_image: loader.bin kernel.bin wkcn1.com wkcn2.com wkcn3.com wkcn4.com kan.com
	dd if=/dev/zero of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 count=2880
	dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=2 conv=notrunc #Shanqu 3
	printf "\x55\xaa" | dd of=$(BUILD_DIR)/$(IMAGE_NAME) bs=1 seek=510 count=2 conv=notrunc
	#User Programs
	dd if=$(BUILD_DIR)/wkcn1.com of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=18 conv=notrunc
	dd if=$(BUILD_DIR)/wkcn2.com of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=19 conv=notrunc
	dd if=$(BUILD_DIR)/wkcn3.com of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=20 conv=notrunc
	dd if=$(BUILD_DIR)/wkcn4.com of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=21 conv=notrunc
	dd if=$(BUILD_DIR)/kan.com of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=22 conv=notrunc


loader.bin: loader.asm
	$(AS) $(ASFLAGS) -f bin $^ -o $(BUILD_DIR)/$@

kernel.bin: kernel.o os.o
	$(LD) $(LDFLAGS) -Ttext 0x7e00 --oformat binary $(BUILD_DIR)/kernel.o $(BUILD_DIR)/os.o -o $(BUILD_DIR)/$@


%.o: %.c
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@ 
%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@ 
%.o: %.asm
	$(AS) $(ASFLAGS) -f elf32 $^ -o $(BUILD_DIR)/$@
%.com: %.asm
	$(AS) $(ASFLAGS) -f bin $^ -o $(BUILD_DIR)/$@
	#$(LD) $(LDFLAGS) -Ttext 0x0100 $(BUILD_DIR)/$^ -o $(BUILD_DIR)/$@ 

qemu:
	$(QEMU) $(QEMUFLAGS) $(BUILD_DIR)/disk.img

run: qemu

bochs:
	$(BOCHS) $(BOCHSFLAGS)

clean:
	-rm -rf build
