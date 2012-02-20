CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I../libc/inc -fno-pic

OBJS=	src/loader.o src/main.o

DISK_IMAGE_SIZE=33546240 # that's 32MB

hdd.img: build/kernel.sys
	cp hdd.base.img hdd.img
	sudo rm -rf mnt
	sudo mkdir mnt
	sudo mount -o loop,offset=512 hdd.img mnt
	sudo cp build/kernel.sys mnt/
	sudo umount mnt
	sudo rm -rf mnt

clean:
	rm -f hdd.img
	rm -f build/*
	rm -f src/*.o

build/kernel.sys: $(OBJS)
	ld -T linker.ld -o build/kernel.sys $(LDFLAGS) $(OBJS)

src/loader.o: src/loader.asm
	nasm -f elf -o src/loader.o src/loader.asm