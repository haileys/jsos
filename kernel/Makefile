CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I../libc/inc

OBJS=	src/loader.o src/main.o

DISK_IMAGE_SIZE=33546240 # that's 32MB

hdd.img:
	# ensure grub is installed
	which grub
	# format the image
	dd if=/dev/zero of=hdd.img bs=$(DISK_IMAGE_SIZE) count=1 status=noxfer
	parted --script hdd.img mklabel msdos		2>/dev/null
	parted --script hdd.img primary fat16 0 31	2>/dev/null
	parted --script hdd.img mkfs 1 fat16		2>/dev/null
	parted --script hdd.img set 1 boot on		2>/dev/null
	# copy files onto image
	bash -c '	if [[ ! -x mnt ]]; then mkdir mnt --mode=0777; \
				else sudo unmount mnt; fi; exit 0'
	sudo mount -o loop,offset=512 hdd.img mnt
	sudo cp -r build/boot mnt/
	sudo cp build/kernel.sys mnt/
	sudo umount mnt
	sudo rm -rf mnt
	# install GRUB
	grub --device-map=/dev/null --batch < grubscript

build/kernel.sys: $(OBJS)
	ld -T linker.ld -o build/kernel.sys $(LDFLAGS) $(OBJS)

src/loader.o: src/loader.asm
	nasm -f elf -o src/loader.o src/loader.asm