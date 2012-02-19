#include <string.h>
#include <stdio.h>
#include "string.h"
#include "gc.h"

js_string_t* js_string_concat(js_string_t* a, js_string_t* b)
{
    js_string_t* str = js_alloc(sizeof(js_string_t));
    str->length = a->length + b->length;
    str->buff = js_alloc(str->length + 1);
    memcpy(str->buff, a->buff, a->length);
    memcpy(str->buff + a->length, b->buff, b->length);
    str->buff[str->length] = 0;
    return str;
}

js_string_t* js_string_format(char* fmt, ...)
{
    js_string_t* retn;
    va_list va;
    va_start(va, fmt);
    retn = js_string_vformat(fmt, va);
    va_end(va);
    return retn;
}

js_string_t* js_string_vformat(char* fmt, va_list args)
{
    js_string_t* str = js_alloc(sizeof(js_string_t));
    str->buff = js_alloc(1024);
    str->length = vsnprintf(str->buff, 1023, fmt, args);
    str->buff[1023] = 0;
    return str;
}