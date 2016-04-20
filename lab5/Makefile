BUILD_DIR = build
DISK_DIR = disk
KITS_DIR = kits
IMAGE_NAME=disk.img
CC = g++
CFLAGS = -Iinclude -c -m16 -march=i386 -masm=intel -nostdlib -ffreestanding -mpreferred-stack-boundary=2 -lgcc -shared -std=c++11 -Wno-int-to-void-pointer-cast -fpermissive

AS = nasm
ASFLAGS =
LD = ld
LDFLAGS = -m elf_i386 -N

#如果不加-fda, 虽然能读Loader, 但无法读扇区:-(
QEMU = qemu-system-i386
QEMUFLAGS = -fda

BOCHS = bochs
BOCHSFLAGS = -q -f bochsrc.bxrc

all: build_dir counter write_image write_knl build_progs


build_dir:
	-mkdir $(BUILD_DIR)
	-mkdir $(DISK_DIR)
	./counter

write_image: loadknl.bin
	dd if=/dev/zero of=$(BUILD_DIR)/disk.img count=2880
	dd if=$(BUILD_DIR)/loadknl.bin of=$(BUILD_DIR)/disk.img conv=notrunc

write_knl: kernel.bin
	mount -o loop $(BUILD_DIR)/disk.img disk/
	cp $(BUILD_DIR)/kernel.bin $(DISK_DIR)/
	umount $(DISK_DIR)/

build_progs: wkcn1.com wkcn2.com wkcn3.com wkcn4.com kan.com\
   			hello.com bs.com bpb.com ls.com help.com\
			box.com ouch.com clock.com\
		   	inttest.com porttest.com fork1.com fork2.com alpha.com

	mount -o loop $(BUILD_DIR)/disk.img disk/

	for name in $^; do\
		cp $(BUILD_DIR)/$$name $(DISK_DIR)/;\
	done

	umount $(DISK_DIR)/

loadknl.bin: loadknl.asm
	$(AS) $(ASFLAGS) -f bin $^ -o $(BUILD_DIR)/$@

kernel.bin: kernel.o os.o
	$(LD) $(LDFLAGS) -Ttext 0x7e00 --oformat binary $(BUILD_DIR)/kernel.o $(BUILD_DIR)/os.o -o $(BUILD_DIR)/$@


%.o: %.c
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@ 
%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@ 
%.o: %.asm
	$(AS) $(ASFLAGS) -f elf32 $^ -o $(BUILD_DIR)/$@
%.com: %.o header.o
	#$(LD) $(LDFLAGS) -Ttext 0x100 --oformat binary $(BUILD_DIR)/header.o $(BUILD_DIR)/$< -o $(BUILD_DIR)/$@
	$(LD) $(LDFLAGS) -T linker_100.ld --oformat binary $(BUILD_DIR)/header.o $(BUILD_DIR)/$< -o $(BUILD_DIR)/$@
%.com: %.asm
	$(AS) $(ASFLAGS) -f bin $^ -o $(BUILD_DIR)/$@
	#$(LD) $(LDFLAGS) -Ttext 0x0100 $(BUILD_DIR)/$^ -o $(DISK_DIR)/$@ 

qemu:
	$(QEMU) $(QEMUFLAGS) $(BUILD_DIR)/disk.img

run: qemu

bochs:
	$(BOCHS) $(BOCHSFLAGS)

build_counter:
	g++ counter.cpp -o counter

clean:
	-rm -rf $(BUILD_DIR)
	-rm -rf $(DISK_DIR)
