#ifndef JS_STRING_H
#define JS_STRING_H

#include "value.h"
#include <stdarg.h>

js_string_t* js_string_concat(js_string_t* a, js_string_t* b);
js_string_t* js_string_format(char* fmt, ...);
js_string_t* js_string_vformat(char* fmt, va_list args);

#endif