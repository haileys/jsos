#ifndef JS_JIT_H
#define JS_JIT_H

#include "image.h"
#include "value.h"

js_native_callback_t js_jit_section(js_section_t* section, uint32_t* length);

#endif