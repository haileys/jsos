#include <string.h>
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

js_string_t* js_string_from_double(double number)
{
    double d = number;
    js_string_t* str = js_alloc(sizeof(js_string_t));
    uint32_t index = 0;
    if(d != d) {
        memcpy(str->buff, "NaN", 4);
        str->length = 3;
        return str;
    }
    if(!isfinite(d)) {
        if(d > 0) {
            memcpy(str->buff, "Infinity", 9);
            str->length = 8;
        } else {
            memcpy(str->buff, "-Infinity", 10);
            str->length = 9;
        }
        return str;
    }
    str->length = 0;
    str->buff = js_alloc(64);
    if(d < 0) {
        str->buff[index++] = '-';
        d = -d;
    }
    double whole = floor(d);
    if(whole != 0) {
        char wholebuff[32];
        uint32_t wholebuffidx = 0;
        while(whole > 0) {
            wholebuff[wholebuffidx++] = '0' + (int)fmod(whole, 10.0);
            whole = floor(whole / 10.0);
        }
        while(wholebuffidx > 0) {
            str->buff[index++] = wholebuff[--wholebuffidx];
        }
    } else {
        str->buff[index++] = '0';
    }
    d -= floor(number);
    if(d > 0) {
        uint32_t frac_digits;
        str->buff[index++] = '.';
        for(frac_digits = 0; frac_digits < 6; frac_digits++) {
            d = fmod(d, 1.0) * 10.0;
            if(d == 0.0) break;
            str->buff[index++] = '0' + (int)floor(d);
        }
    }
    str->buff[index] = 0;
    str->length = index;
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