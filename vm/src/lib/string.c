#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "lib.h"
#include "object.h"
#include "gc.h"
#include "exception.h"

typedef struct {
    js_value_t base;
    js_string_t string;
} js_string_object_t;

static VAL String_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_value_make_cstring("");
    } else {
        return js_to_string(argv[0]);
    }
}

static VAL String_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_make_string_object(vm, js_cstring(""));
    } else {
        js_value_t* val = js_value_get_pointer(js_to_string(argv[1]));
        return js_make_string_object(vm, &val->string);
    }
}

VAL js_make_string_object(js_vm_t* vm, js_string_t* string)
{
    js_string_object_t* str = js_alloc(sizeof(js_string_object_t));
    str->base.type = JS_T_STRING_OBJECT;
    str->base.object.vtable = js_object_base_vtable();
    str->base.object.prototype = vm->lib.String_prototype;
    str->base.object.class = vm->lib.String;
    str->base.object.properties = js_st_table_new();
    str->string.buff = string->buff;
    str->string.length = string->length;
    VAL v = js_value_make_pointer((js_value_t*)str);
    js_object_put(v, js_cstring("length"), js_value_make_double(str->string.length));
    return v;
}

static VAL String_fromCharCode(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_value_t* val = js_alloc(sizeof(js_value_t));
    val->type = JS_T_STRING;
    val->string.length = argc;
    uint32_t i;
    for(i = 0; i < argc; i++) {
        val->string.buff[i] = (uint8_t)js_value_get_double(js_to_number(argv[i]));
    }
    return js_value_make_pointer(val);
}

static VAL String_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.toString() is not generic");
    }
    js_string_object_t* strobj = ((js_string_object_t*)js_value_get_pointer(this));
    return js_value_wrap_string(&strobj->string);
}

static VAL String_prototype_valueOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.valueOf() is not generic");
    }
    js_string_object_t* strobj = ((js_string_object_t*)js_value_get_pointer(this));
    return js_value_wrap_string(&strobj->string);
}

static VAL String_prototype_substr(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.substr() is not generic");
    }
    js_string_object_t* str = (js_string_object_t*)js_value_get_pointer(this);
    if(argc == 0) {
        return this;
    }
    if(argc == 1) {
        uint32_t index = js_to_uint32(argv[0]);
        if(index >= str->string.length) {
            return js_value_make_cstring("");
        }
        return js_value_make_string(str->string.buff + index, str->string.length - index);
    }
    uint32_t index = js_to_uint32(argv[0]);
    uint32_t length = js_to_uint32(argv[1]);
    if(index >= str->string.length) {
        return js_value_make_cstring("");
    }
    if(index + length >= str->string.length) {
        length = str->string.length - index;
    }
    return js_value_make_string(str->string.buff + index, length);
}

static VAL String_prototype_trimRight(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.substr() is not generic");
    }
    js_string_object_t* str = (js_string_object_t*)js_value_get_pointer(this);
    uint32_t new_len = str->string.length;
    while(str->string.buff[new_len - 1] == ' ') {
        if(--new_len == 0) {
            return js_value_make_cstring("");
        }
    }
    return js_value_make_string(str->string.buff, new_len);
}

void js_lib_string_initialize(js_vm_t* vm)
{
    vm->lib.String = js_value_make_native_function(vm, NULL, js_cstring("String"), String_call, String_construct);
    js_object_put(vm->global_scope->global_object, js_cstring("String"), vm->lib.String);
    
    vm->lib.String_prototype = js_value_make_object(vm->lib.Object_prototype, vm->lib.String);
    js_object_put(vm->lib.String, js_cstring("prototype"), vm->lib.String_prototype);
    js_object_put(vm->lib.String, js_cstring("fromCharCode"), js_value_make_native_function(vm, NULL, js_cstring("fromCharCode"), String_fromCharCode, NULL));
    
    js_object_put(vm->lib.String_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), String_prototype_toString, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("valueOf"), js_value_make_native_function(vm, NULL, js_cstring("valueOf"), String_prototype_valueOf, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("substr"), js_value_make_native_function(vm, NULL, js_cstring("substr"), String_prototype_substr, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("trimRight"), js_value_make_native_function(vm, NULL, js_cstring("trimRight"), String_prototype_trimRight, NULL));
}