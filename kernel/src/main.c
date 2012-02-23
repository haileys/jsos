#include "multiboot.h"
#include <stdio.h>
#include <string.h>

void write_string(char* s)
{
    uint8_t* vram = (uint8_t*)0xb8000;
    size_t i, len = strlen(s);
    for(i = 0; i < len; i++) {
        vram[i * 2] = s[i];
        vram[i * 2 + 1] = 15;
    }
}

void kmain(struct multiboot_info* mbd, uint32_t magic)
{
    char buff[100];
    snprintf(buff, 100, "Hello %s!", "world");
    write_string(buff);
}