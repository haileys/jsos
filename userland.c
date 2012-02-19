#include <stdlib.h>
#include <stdio.h>

__asm__(".globl _start \n _start: \n jmp main");

int main()
{
    printf("Hello, world!\n");
    exit(0);
    return 0;
}