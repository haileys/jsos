#include <stdio.h>
#include <stdarg.h>
#include "panic.h"
#include "console.h"

void panic(char* message)
{
    kprintf("\n\nKernel Panic - %s\n", message);
    __asm__ volatile ("cli \n hlt");
    for(;;); // make gcc shutup
}

void panicf(char* fmt, ...)
{
    char buff[2048];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buff, 2047, fmt, va);
    va_end(va);
    panic(buff);
}

void js_panic_handler(const char* func, char* file, int line, char* message)
{
    panicf("%s, %s() in %s:%d", message, func, file, line);
}