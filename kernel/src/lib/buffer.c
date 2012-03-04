#include <value.h>
#include <vm.h>
#include <gc.h>
#include <exception.h>
#include <string.h>
#include "console.h"
#include "lib.h"

static VAL Buffer;
static VAL Buffer_prototype;

typedef struct {
    char* buffer;
    uint32_t size;
    uint32_t capacity;
} buffer_object_t;

static buffer_object_t* get_buffer(js_vm_t* vm, VAL this)
{
    if(js_value_get_type(this) != JS_T_OBJECT || js_value_get_pointer(js_value_get_pointer(this)->object.class) != js_value_get_pointer(Buffer)) {
        js_throw_error(vm->lib.TypeError, "Can't call Buffer instance methods with non-Buffer as object");
    }
    return (buffer_object_t*)js_value_get_pointer(this)->object.state;
}

static VAL Buffer_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    buffer_object_t* buffer = js_alloc(sizeof(buffer_object_t));
    buffer->capacity = 16;
    if(argc > 0 && js_value_get_type(argv[0]) == JS_T_NUMBER) {
        buffer->capacity = js_to_uint32(argv[0]);
    }
    buffer->buffer = js_alloc_no_pointer(buffer->capacity);
    buffer->size = 0;
    js_value_get_pointer(this)->object.state = buffer;
    return js_value_undefined();
}

static VAL Buffer_prototype_append(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    buffer_object_t* buffer = get_buffer(vm, this);
    uint32_t i;
    uint32_t new_size = buffer->size;
    for(i = 0; i < argc; i++) {
        if(js_value_get_type(argv[i]) == JS_T_STRING) {
            new_size += js_value_get_pointer(argv[i])->string.length;
        } else if(js_value_get_type(argv[i]) == JS_T_OBJECT && js_value_get_pointer(js_value_get_pointer(argv[i])->object.class) != js_value_get_pointer(Buffer)) {
            new_size += ((buffer_object_t*)js_value_get_pointer(this)->object.state)->size;
        } else {
            js_throw_error(vm->lib.TypeError, "Expected either string or Buffer as argument %d", i + 1);
        }
    }
    while(buffer->capacity < new_size) {
        buffer->capacity *= 2;
    }
    buffer->buffer = js_realloc(buffer->buffer, buffer->capacity);
    for(i = 0; i < argc; i++) {
        void* ptr;
        uint32_t sz;
        if(js_value_get_type(argv[i]) == JS_T_STRING) {
            ptr = js_value_get_pointer(argv[i])->string.buff;
            sz = js_value_get_pointer(argv[i])->string.length;
        } else if(js_value_get_type(argv[i]) == JS_T_OBJECT && js_value_get_pointer(js_value_get_pointer(argv[i])->object.class) != js_value_get_pointer(Buffer)) {
            ptr = ((buffer_object_t*)js_value_get_pointer(this)->object.state)->buffer;
            sz = ((buffer_object_t*)js_value_get_pointer(this)->object.state)->size;
        }
        memcpy(buffer->buffer + buffer->size, ptr, sz);
        buffer->size += sz;
    }
    return this;
}

static VAL Buffer_prototype_get_contents(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    buffer_object_t* buffer = get_buffer(vm, this);
    return js_value_make_string(buffer->buffer, buffer->size);
}

void lib_buffer_init(js_vm_t* vm)
{
    Buffer = js_value_make_native_function(vm, NULL, js_cstring("Buffer"), NULL, Buffer_construct);
    js_gc_register_global(&Buffer, sizeof(Buffer));
    Buffer_prototype = js_object_get(Buffer, js_cstring("prototype"));
    js_gc_register_global(&Buffer_prototype, sizeof(Buffer_prototype));
    js_object_put(vm->global_scope->global_object, js_cstring("Buffer"), Buffer);
    
    // instance methods:
    js_object_put(Buffer_prototype, js_cstring("append"), js_value_make_native_function(vm, NULL, js_cstring("append"), Buffer_prototype_append, NULL));
    js_object_put(Buffer_prototype, js_cstring("getContents"), js_value_make_native_function(vm, NULL, js_cstring("getContents"), Buffer_prototype_get_contents, NULL));
}