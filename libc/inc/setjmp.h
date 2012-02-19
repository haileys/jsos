#ifndef LIBC_SETJMP_H
#define LIBC_SETJMP_H

#include "stdint.h"

struct __jmp_buf_tag {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eip;
    uint32_t eflags;
} __attribute__((packed));

typedef struct __jmp_buf_tag jmp_buf[1];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int value);

#endif