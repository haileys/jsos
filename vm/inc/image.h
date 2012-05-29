#ifndef JS_IMAGE_H
#define JS_IMAGE_H

#include <stdint.h>
#include "value.h"

#define JS_FLAG_HAS_INNER_FUNCS (1)

typedef struct {
    uint32_t instruction_count;
    uint32_t flags;
    uint32_t var_count;
    uint32_t* instructions;
} js_section_t;

typedef struct js_image {
    uint32_t signature;
    uint32_t name;
    uint32_t section_count;
    js_section_t* sections;
    uint32_t string_count;
    js_string_t** strings;
} js_image_t;

js_image_t* js_image_parse(char* buff, uint32_t buff_size);

#endif