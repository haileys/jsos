#include <stdlib.h>
#include <stdio.h>

__asm__(".globl start \n start: \n jmp _main");

int main()
{
    printf("Hello, world!\n");
    exit(0);
    return 0;
}