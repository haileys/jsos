#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "gc.h"

/* this function is insecure. todo: sprinkle some more bounds checks through */

#define CHECK_AHEAD(bytes) if(buff + (bytes) > buff_end) return NULL

js_image_t* js_image_parse(char* buff, uint32_t buff_size)
{
    uint32_t i, sz;
    char* buff_end = buff + buff_size;
    js_image_t* image;
    
    image = js_alloc(sizeof(js_image_t));
    CHECK_AHEAD(12);
    memcpy(image, buff, 12);
    if(image->signature != 0x0058534a /* "JSX\0" */) {
        return NULL;
    }
    buff += 12;
    image->sections = js_alloc(sizeof(js_section_t) * image->section_count);
    for(i = 0; i < image->section_count; i++) {
        CHECK_AHEAD(4);
        sz = *(uint32_t*)buff;
        image->sections[i].instruction_count = sz / 4;
        buff += 4;
        CHECK_AHEAD(4);
        image->sections[i].flags = *(uint32_t*)buff;
        buff += 4;
        CHECK_AHEAD(4);
        image->sections[i].var_count = *(uint32_t*)buff;
        buff += 4;
        CHECK_AHEAD(sz);
        image->sections[i].instructions = js_alloc_no_pointer(sz);
        memcpy(image->sections[i].instructions, buff, sz);
        buff += sz;
    }
    CHECK_AHEAD(4);
    image->string_count = *(uint32_t*)buff;
    image->strings = js_alloc(sizeof(js_string_t*) * image->string_count);
    buff += 4;
    for(i = 0; i < image->string_count; i++) {
        CHECK_AHEAD(4);
        sz = *(uint32_t*)buff;
        buff += 4;
        image->strings[i] = js_alloc(sizeof(js_string_t));
        image->strings[i]->length = sz;
        image->strings[i]->buff = js_alloc_no_pointer(sz + 1);
        CHECK_AHEAD(sz + 1);
        memcpy(image->strings[i]->buff, buff, sz + 1);
        buff += sz + 1;
    }
    return image;
}