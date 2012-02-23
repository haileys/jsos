#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include "stdarg.h"
#include "stddef.h"

int printf(char* fmt, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);

#endif