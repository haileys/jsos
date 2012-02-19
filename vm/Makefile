CFLAGS=-m32 -Wall -Wextra -iquote inc -Wno-unused-parameter -g -DJS_GC_DEBUG
LDFLAGS=
OBJS=	src/image.o src/scope.o src/st.o src/value.o src/vm.o src/object.o \
		src/string.o src/gc.o src/lib.o src/lib/array.o src/lib/function.o \
		src/lib/object.o src/lib/number.o src/lib/error.o src/exception.o

all: disasm runner

disasm: $(OBJS)

runner: $(OBJS)

gctest: src/gc.o

clean:
	rm -f src/*.o
	rm -f src/*/*.o
	rm -f runner
	rm -f disasm
	rm -f gctest