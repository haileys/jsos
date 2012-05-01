CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I../libc/inc -I../vm/inc -fno-pic -Werror \
		-DJS_GC_DEBUG -std=c99 -g

LDFLAGS=-melf_i386 ../libc/src/*.o ../vm/libjsvm.a

OBJS=	src/loader.o src/main.o src/mm.o src/panic.o src/malloc.o src/console.o \
		src/helpers.o src/gdt.o src/interrupt.o src/isrs.o src/io.o \
		src/lib/binary_utils.o src/lib/kernel.o src/lib/vm.o src/realmode.o \
		src/lib/buffer.o

JSIMGS=	kernel/init kernel/keyboard kernel/drivers/ps2kb \
		kernel/drivers/ide kernel/drivers/ps2mouse kernel/drivers/fat16 \
		kernel/drivers/serial kernel/drivers/mbr kernel/drivers/vga \
		kernel/drivers/pit kernel/drivers/rtc kernel/utils \
		kernel/process kernel/drivers/bios_hdd kernel/vm kernel/fs \
		bin/init

.SUFFIXES: .js .o .asm .o .coffee .js

.js.o:
	@echo "      js  $<"
	@ruby ../compiler/compile.rb $< > $@

.coffee.o:
	@echo "  coffee  $<"
	@coffee -cs < $< | ruby ../compiler/compile.rb /dev/stdin > $@

.asm.o:
	@echo "    nasm  $<"
	@nasm -f elf -o $@ $<

%.o: %.c
	@echo "      cc  $<"
	@${CC} ${CFLAGS} -c -o $@ $<

hdd.img: $(patsubst %, build/%.jmg, $(JSIMGS)) build/kernel.sys \
	 	build/boot/grub/menu.lst build/jsos.256
	@echo "creating image"
	@cp hdd.base.img hdd.img
	@sudo rm -rf mnt
	@sudo mkdir mnt
	@sudo mount -o loop,offset=512 hdd.img mnt
	@sudo rm -f mnt/kernel/*.jmg
	@sudo rm -f mnt/kernel/*/*.jmg
	@sudo cp -R build/* mnt/
	@sudo umount mnt
	@sudo rm -rf mnt

clean:
	rm -f hdd.img
	rm -f build/kernel.sys
	rm -f build/kernel/*.jmg
	rm -f build/kernel/*/*.jmg
	rm -f build/*.256
	rm -f src/*/*.o
	rm -f src/*.o
	rm -f js/*.o

src/realmode.o: src/realmode.asm
	@echo "    nasm  src/realmode.asm"
	@nasm -f bin -o src/realmode.bin $<
	@objcopy --input binary --output elf32-i386 --binary-architecture i386 src/realmode.bin $@
	@rm src/realmode.bin

build/kernel.sys: $(OBJS) ../vm/libjsvm.a
	@echo "      ld"
	@$(LD) -T linker.ld -o build/kernel.sys $(OBJS) $(LDFLAGS)

build/%.jmg: js/%.o
	@cp $< $@

build/%.256: %.png
	@echo " convert $<"
	@convert $< -depth 8 tmp.rgb
	@ruby scripts/rgb-to-256indexed.rb tmp.rgb > $@
	@rm -f tmp.rgb