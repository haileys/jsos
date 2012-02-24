#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include "stdarg.h"
#include "stdlib.h"

size_t strlen(const char* str);
int strcmp(const char* a, const char* b);

void* memset(void* ptr, char c, size_t bytes);
void* memcpy(void* dest, const void* source, size_t bytes);
void* memmove(void* dest, const void* source, size_t bytes);
int memcmp(const void* a, const void* b, size_t bytes);

#endif