CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -O3 #-DJS_GC_DEBUG
LDFLAGS=
OBJS=	src/image.o src/scope.o src/st.o src/value.o src/vm.o src/object.o \
		src/string.o src/gc.o src/lib.o src/lib/array.o src/lib/function.o \
		src/lib/object.o src/lib/number.o src/lib/error.o src/exception.o

libjsvm.a: CFLAGS += -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-stack-protector -I../libc/inc/ -static -fno-pic

all: disasm runner

libjsvm.a: clean $(OBJS)
	ar rcs libjsvm.a $(OBJS)

disasm: $(OBJS)

runner: $(OBJS)

clean:
	rm -f src/*.o
	rm -f src/*/*.o
	rm -f runner
	rm -f disasm
	rm -f gctest
	rm -f *.a