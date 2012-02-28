#ifndef CONSOLE_H
#define CONSOLE_H

#include <vm.h>

void console_init(js_vm_t* vm);
void console_clear();
void console_write(char* s, size_t length);
void console_puts(char* s);
void console_cursor(int row, int col);
void kprintf(char* fmt, ...);

#endif