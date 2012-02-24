#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "io.h"
#include "console.h"

static int row, col;

void console_clear()
{
    memset((void*)0xb8000, 0, 80*25*2);
    col = 0;
    row = 0;
}

void console_puts(char* s)
{
    char* vram = (char*)0xb8000;
    for(; *s; s++) {
        if(*s != '\n') {
            vram[(row * 80 + col) * 2] = *s;
            vram[(row * 80 + col) * 2 + 1] = 7;
            col++;
        } else {
            col = 0;
            row++;
        }
        if(col == 80) {
            col = 0;
            row++;
        }
        if(row == 25) {
            memmove(vram, vram + 80 * 2, 24 * 80 * 2);
            memset(vram + 24 * 80 * 2, 0, 80 * 2);
            row = 24;
        }
    }
    console_cursor(row, col);
}

void console_cursor(int r, int c)
{
    uint16_t pos, base_vga_port;
    row = r;
    col = c;
    pos = row * 80 + col;
    base_vga_port = *(uint16_t*)0x463; // read base vga port from bios data area
    
    outb(base_vga_port, 0x0e);
    outb(base_vga_port + 1, (pos >> 8) & 0xff);
    
    outb(base_vga_port, 0x0f);
    outb(base_vga_port + 1, pos & 0xff);
}

void kprintf(char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buff[1024];
    vsnprintf(buff, 1024, fmt, va);
    va_end(va);
    console_puts(buff);
}
