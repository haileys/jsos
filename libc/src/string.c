#include "string.h"

size_t strlen(const char* str)
{
    size_t len = 0;
    while(*str++) len++;
    return len;
}

int strcmp(const char* a, const char* b)
{
    while(1) {
        if(*a == 0 && *b == 0) {
            return 0;
        }
        if(*a != *b) {
            return *a - *b;
        }
        a++;
        b++;
    }
}

void* memset(void* ptr, char c, size_t bytes)
{
    char* p = (char*)ptr;
    size_t i;
    for(i = 0; i < bytes; i++) {
        p[i] = c;
    }
    return ptr;
}

void* memcpy(void* dest, const void* source, size_t bytes)
{
    char* d = (char*)dest;
    const char* s = (const void*)source;
    size_t i;
    for(i = 0; i < bytes; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memmove(void* dest, const void* source, size_t bytes)
{
    char* d = (char*)dest;
    const char* s = (const void*)source;
    size_t i;
    if(d < s) {
        for(i = 0; i < bytes; i++) {
            d[i] = s[i];
        }
    } else {
        for(i = bytes; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

int memcmp(const void* a, const void* b, size_t bytes)
{
    const char* ca = (const char*)a;
    const char* cb = (const char*)b;
    size_t i;
    for(i = 0; i < bytes; i++) {
        if(ca[i] != cb[i]) {
            return ca[i] - cb[i];
        }
    }
    return 0;
}

void* memchr(void* ptr, int value, size_t bytes)
{
    size_t i;
    uint8_t* p = (uint8_t*)ptr;
    uint8_t c = (uint8_t)value;
    for(i = 0; i < bytes; i++, p++) {
        if(*p == c) {
            return p;
        }
    }
    return NULL;
}