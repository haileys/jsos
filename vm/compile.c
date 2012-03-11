#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "image.h"
#include "vm.h"
#include "value.h"
#include "gc.h"
#include "jit.h"

char* read_until_eof(FILE* f, uint32_t* len)
{
    size_t cap = 4096;
    size_t idx = 0;
    char* buff = js_alloc(cap);
    while(!feof(stdin)) {
        idx += fread(buff + idx, 1, 4096, f);
        if(idx >= cap) {
            cap *= 2;
            buff = js_realloc(buff, cap);
        }
    }
    *len = idx;
    return buff;
}

int main()
{
    uint32_t dummy;
    uint32_t len;
    char* buff;
    js_image_t* image;
    
    js_gc_init(&dummy);
    buff = read_until_eof(stdin, &len);
    image = js_image_parse(buff, len);
    
    uint32_t length;
    void* x86 = js_jit_section(&image->sections[0], &length);
    fwrite(x86, 1, length, stdout);
    
    return 0;
}