#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "gc.h"
#include "value.h"

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

bool js_string_eq(js_string_t* a, js_string_t* b)
{
    if(a->length != b->length) {
        return false;
    }
    return memcmp(a->buff, b->buff, a->length) == 0;
}

js_string_t* js_string_from_double(double number)
{
    if(number != number) {
        return js_cstring("NaN");
    }
    if(!isfinite(number)) {
        if(number > 0) {
            return js_cstring("Infinity");
        } else {
            return js_cstring("-Infinity");
        }
    }
    double whole = floor(number);
    double frac = fabs(number) - fabs(whole);
    char buff[200];
    char* ptr = buff;
    if(whole < 0) {
        *ptr++ = '-';
        whole = -number;
    }
    if(whole == 0) {
        *ptr++ = '0';
    }
    char minibuff[200];
    memset(minibuff, 0, 200);
    uint32_t minibuff_i = 0;
    while(whole > 0) {
        minibuff[minibuff_i++] = '0' + (int)fmod(whole, 10.0);
        whole = floor(whole / 10.0);
    }
    while(minibuff_i > 0) {
        *ptr++ = minibuff[--minibuff_i];
    }
    uint32_t frac_i;
    if(frac > 0) {
        *ptr++ = '.';
        for(frac_i = 0; frac_i < 6; frac_i++) {
            frac = fmod(frac * 10.0, 10.0);
            *ptr++ = '0' + (int)floor(frac);
        }
    }
    *ptr++ = 0;
    return js_cstring(buff);
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