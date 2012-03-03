CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g -nostdlib \
		-nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions \
		-fno-stack-protector -static -fno-pic -Werror
OBJS=	src/string.o src/setjmp.o src/stdio.o src/stdlib.o src/math.o

libc.a: $(OBJS)
	@echo "creating libc.a"
	@ar rcs libc.a $(OBJS)

src/setjmp.o: src/setjmp.asm
	@echo "    nasm  $<"
	@nasm -f elf32 -o src/setjmp.o src/setjmp.asm

# these CFLAGS are the same as the CFLAGS up the top except -fno-builtin is removed
src/math.o: CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g \
			-nostdlib -nostdinc -nostartfiles -nodefaultlibs -fno-exceptions \
			-fno-stack-protector


%.o: %.c
	@echo "      cc  $<"
	@${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f src/*.o
	rm -f *.a