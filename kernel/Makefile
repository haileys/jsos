CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I../libc/inc -I../vm/inc -fno-pic -Werror \
		-DJS_GC_DEBUG

LDFLAGS=../libc/src/*.o ../vm/src/*.o ../vm/src/*/*.o

OBJS=	src/loader.o src/main.o src/mm.o src/panic.o src/malloc.o src/console.o \
		src/helpers.o src/gdt.o src/interrupt.o src/isrs.o src/io.o \
		src/lib/binary_utils.o src/lib/kernel.o src/lib/vm.o

JSIMGS=	init keyboard keymaps drivers drivers/ps2kb drivers/ide drivers/ps2mouse

.SUFFIXES: .js .o .asm .o .coffee .js

.js.o:
	ruby ../compiler/compile.rb $< > $@

.coffee.o:
	coffee -cs < $< | ruby ../compiler/compile.rb /dev/stdin > $@

.asm.o:
	nasm -f elf -o $@ $<

hdd.img: build/kernel.sys
	cp hdd.base.img hdd.img
	sudo rm -rf mnt
	sudo mkdir mnt
	sudo mount -o loop,offset=512 hdd.img mnt
	sudo rm -f mnt/kernel/*.jmg
	sudo rm -f mnt/kernel/*/*.jmg
	sudo cp -R build/* mnt/
	sudo umount mnt
	sudo rm -rf mnt

clean:
	rm -f hdd.img
	rm -f build/kernel.sys
	rm -f build/kernel/*.jmg
	rm -f build/kernel/*/*.jmg
	rm -f src/*/*.o
	rm -f src/*.o
	rm -f js/*.o

build/kernel.sys: $(OBJS) build/boot/grub/menu.lst $(patsubst %, build/kernel/%.jmg, $(JSIMGS))
	$(LD) -T linker.ld -o build/kernel.sys $(OBJS) $(LDFLAGS)

build/kernel/%.jmg: js/%.o
	cp $< $@