#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <value.h>
#include <lib.h>
#include <gc.h>
#include "io.h"
#include "console.h"

static uint32_t row, col, width = 80, height = 25;

static VAL Console_clear(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    console_clear();
    return js_value_undefined();
}

static VAL Console_write(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_string_t* str = js_to_js_string_t(argc ? argv[0] : js_value_undefined());
    console_write(str->buff, str->length);
    return js_value_undefined();
}

static VAL Console_cursor(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t r, c;
    if(argc == 0) {
        VAL* yx = js_alloc(sizeof(VAL) * 2);
        yx[0] = js_value_make_double(row);
        yx[1] = js_value_make_double(col);
        return js_make_array(vm, 2, yx);
    }
    js_scan_args(vm, argc, argv, "II", &r, &c);
    if(r >= height) {
        row = height - 1;
    }
    if(c >= width) {
        col = width - 1;
    }
    console_cursor(r, c);
    return js_value_undefined();
}

static VAL Console_set_size(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t w, h;
    js_scan_args(vm, argc, argv, "II", &w, &h);
    width = w;
    height = h;
    return js_value_undefined();
}

void console_init(js_vm_t* vm)
{
    VAL Console = js_make_object(vm);
    js_object_put(vm->global_scope->global_object, js_cstring("Console"), Console);
    js_object_put(Console, js_cstring("clear"), js_value_make_native_function(vm, NULL, js_cstring("clear"), Console_clear, NULL));
    js_object_put(Console, js_cstring("write"), js_value_make_native_function(vm, NULL, js_cstring("write"), Console_write, NULL));
    js_object_put(Console, js_cstring("cursor"), js_value_make_native_function(vm, NULL, js_cstring("cursor"), Console_cursor, NULL));
    js_object_put(Console, js_cstring("setSize"), js_value_make_native_function(vm, NULL, js_cstring("setSize"), Console_set_size, NULL));
}

void console_clear()
{
    memset((void*)0xb8000, 0, width*height*2);
    col = 0;
    row = 0;
}

void console_write(char* s, size_t length)
{
    char* vram = (char*)0xb8000;
    size_t i;
    for(i = 0; i < length; i++) {
        if(s[i] != '\n') {
            vram[(row * width + col) * 2] = s[i];
            vram[(row * width + col) * 2 + 1] = 7;
            col++;
        } else {
            col = 0;
            row++;
        }
        if(col == width) {
            col = 0;
            row++;
        }
        if(row == height) {
            memmove(vram, vram + width * 2, (height - 1) * width * 2);
            memset(vram + (height - 1) * width * 2, 0, width * 2);
            row = height - 1;
        }
    }
    console_cursor(row, col);
}

void console_puts(char* s)
{
    console_write(s, strlen(s));
}

void console_cursor(int r, int c)
{
    uint16_t pos, base_vga_port;
    row = r;
    col = c;
    pos = row * width + col;
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
