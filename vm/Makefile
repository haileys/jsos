CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -Werror -DJS_GC_DEBUG -lm -O3
LDFLAGS=
OBJS=	src/image.o src/scope.o src/st.o src/value.o src/vm.o src/object.o \
		src/string.o src/gc.o src/lib.o src/lib/array.o src/lib/function.o \
		src/lib/object.o src/lib/number.o src/lib/error.o src/exception.o \
		src/lib/string.o src/lib/math.o src/jit.o

libjsvm.a: CFLAGS += -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-stack-protector -I../libc/inc/ -static -fno-pic -DJSOS

libjsvm.a: $(OBJS)
	@echo "creating libjsvm.a"
	@ar rcs libjsvm.a $(OBJS)

disasm: $(OBJS)

runner: $(OBJS) 

compile: $(OBJS)

%.o: %.c Makefile
	@echo "      cc  $<"
	@${CC} ${CFLAGS} -c -o $@ $<

clean:
	@rm -f src/*.o
	@rm -f src/*/*.o
	@rm -f runner
	@rm -f gctest
	@rm -f *.a