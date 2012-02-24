#ifndef CONSOLE_H
#define CONSOLE_H

void console_clear();
void console_puts(char* s);
void console_cursor(int row, int col);
void kprintf(char* fmt, ...);

#endif