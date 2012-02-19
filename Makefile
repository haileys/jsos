CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -I./libc/inc

userland: userland.o libc/libc.a
	ld -o userland -arch i386 libc/libc.a userland.o

libc/libc.a:
	make -C libc libc.a

clean:
	rm -f userland
	rm -f *.o
	make -C libc clean