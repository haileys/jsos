#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define MAX_ALLOC 1024*1024*16
char memory[MAX_ALLOC];
uint32_t i;

void* malloc(size_t size)
{
    if(i + size > MAX_ALLOC) {
        return NULL;
    }
    uint32_t* ptr = (uint32_t*)&memory[i];
    ptr[1] = size;
    i = ((i + size + 8 + 7) / 8) * 8;
    printf("allocating at 0x%x\n", ptr + 2);
    return ptr + 2;
}

void* realloc(void* pointer, size_t size)
{
    printf("realloc!\n");
    /*
    size_t old_size = ((uint32_t*)pointer)[-1];
    void* ptr = malloc(size);
    memcpy(ptr, pointer, size > old_size ? old_size : size);*/
    return NULL;
}

void free(void* pointer)
{
    
}