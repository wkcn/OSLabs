BUILD_DIR = build
IMAGE_NAME=disk.img
CC = gcc
CFLAGS = -c -m16 -march=i386 -masm=intel -nostdlib -ffreestanding
OBJCOPY=objcopy

AS = nasm
ASFLAGS =
LD = ld
LDFLAGS = -m elf_i386

all: clean build_dir write_image

build_dir:
	-mkdir $(BUILD_DIR)

write_image: loader.bin os.bin
	dd if=/dev/zero of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 count=2880
	dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/$(IMAGE_NAME) bs=1 count=512 conv=notrunc
	dd if=$(BUILD_DIR)/os.bin of=$(BUILD_DIR)/$(IMAGE_NAME) bs=512 seek=2 conv=notrunc #Shanqu 3
	printf "\x55\xaa" | dd of=$(BUILD_DIR)/$(IMAGE_NAME) bs=1 seek=510 count=2 conv=notrunc

loader.bin: loader.asm
	$(AS) $(ASFLAGS) -f bin $^ -o $(BUILD_DIR)/$@

os.bin: os.o
	$(LD) $(LDFLAGS) -Tlinker_7e00.ld $(BUILD_DIR)/os.o -o $(BUILD_DIR)/os.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin

%.o: %.c
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@ 
%.o: %.asm
	$(AS) $(ASFLAGS) $^ -o $(BUILD_DIR)/$@
%.com: %.o
	$(LD) $(LDFLAGS) -Ttext 0x0100 $(BUILD_DIR)/$^ -o $(BUILD_DIR)/$@ 

clean:
	-rm -rf build
