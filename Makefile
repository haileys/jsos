CFLAGS=-m32 -Wall -Wextra -iquote inc -g -Wno-unused-parameter -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I./libc/inc -I./vm/inc

LDFLAGS=-nostdlib -static

all: twostroke/LICENSE libc/libc.a vm/libjsvm.a kernel/hdd.img \
	 docs/syscalls.md

twostroke/LICENSE:
	git submodule update --init

libc/libc.a:
	@make -C libc libc.a

vm/libjsvm.a:
	@make -C vm libjsvm.a

kernel/hdd.img:
	@make -C kernel hdd.img

.PHONY: libc/libc.a vm/libjsvm.a kernel/hdd.img

docs/syscalls.md: kernel/js/kernel/process.js scripts/generate-syscall-docs.rb
	@ruby scripts/generate-syscall-docs.rb > $@
	@echo "     doc  $@"

clean:
	rm -f userland
	rm -f *.o
	rm -f docs/*.md
	make -C libc clean
	make -C vm clean
	make -C kernel clean