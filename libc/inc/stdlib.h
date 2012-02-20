#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

#include "stddef.h"

char* itoa(int value, char* buff, int base);
char* utoa(unsigned int value, char* buff, int base);
int atoi(char* str);

void* malloc(size_t size);
void* realloc(void* pointer, size_t size);
void free(void* pointer);

void exit(int)
#ifdef __GNUC__
    __attribute__ ((noreturn))
#endif
;

#endif