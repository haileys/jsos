#include "stdlib.h"
#include "string.h"

char* itoa(int value, char* buff, int base)
{
    char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* ret = buff;
    char scratch[64];
    int idx = 0;
    if(value < 0) {
        *buff++ = '-';
        value = -value;
    }
    if(value == 0) {
        *buff++ = '0';
        *buff = 0;
        return ret;
    }
    while(value > 0) {
        scratch[idx++] = charset[value % base];
        value /= base;
    }
    while(idx > 0) {
        *buff++ = scratch[--idx];
    }
    *buff = 0;
    return ret;
}

int atoi(char* str)
{
    int i = 0;
    int factor = 1;
    if(*str == '-') {
        factor = -1;
        str++;
    }
    if(*str == '+') str++;
    while(*str) {
        if(*str < '0' || *str > '9') break;
        i *= 10;
        i += *str - '0';
        str++;
    }
    return i * factor;
}

void exit(int code)
{
    #ifdef __APPLE__
        __asm__ volatile("andl $0xFFFFFFF0, %esp \n pushl $0 \n movl $1, %eax \n pushl $0 \n int $0x80\n");
    #else
        #ifdef __linux__
            __asm__ volatile("movl $0, %ebx \n movl $1, %eax \n int $0x80");
        #else
            #error exit() not implemented
        #endif
    #endif
    // this is never reached, but it's useful to shutup gcc's "warning: ‘noreturn’ function does return"
    for(;;);
}