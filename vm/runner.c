#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "image.h"
#include "vm.h"
#include "value.h"
#include "gc.h"
#include "exception.h"

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

static VAL console_log(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t i;
    for(i = 0; i < argc; i++) {
        printf("%s%s", i ? " " : "", js_value_get_pointer(js_to_string(argv[i]))->string.buff);
    }
    printf("\n");
    return js_value_undefined();
}

int main()
{
    uint32_t dummy;
    uint32_t len;
    char* buff;
    js_image_t* image;
    js_vm_t* vm;
    VAL exception;
    
    js_gc_init(&dummy);
    buff = read_until_eof(stdin, &len);
    image = js_image_parse(buff, len);
    vm = js_vm_new();
    
    VAL console = js_value_make_object(js_value_null(), js_value_null());
    js_object_put(console, js_cstring("log"), js_value_make_native_function(vm, NULL, js_cstring("log"), console_log, NULL));
    js_object_put(vm->global_scope->global_object, js_cstring("console"), console);
    
    JS_TRY({
        js_vm_exec(vm, image, 0, vm->global_scope, js_value_null(), 0, NULL);
    }, exception, {
        fprintf(stderr, "Unhandled exception: %s\n", js_value_get_pointer(js_to_string(exception))->string.buff);
        exit(-1);
    });
    
    return 0;
}