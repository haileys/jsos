#ifndef PANIC_H
#define PANIC_H

void panic(char* message) __attribute__((noreturn));
void panicf(char* fmt, ...) __attribute__((noreturn));
void js_panic_handler(const char* func, char* file, int line, char* message) __attribute__((noreturn));

#endif