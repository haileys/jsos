#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "string.h"
#include "lib.h"
#include "object.h"
#include "gc.h"
#include "exception.h"

typedef struct {
    js_value_t base;
    js_string_t string;
} js_string_object_t;

static bool statically_initialized;
static js_object_internal_methods_t string_vtable;

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
    str->base.object.vtable = &string_vtable;
    str->base.object.prototype = vm->lib.String_prototype;
    str->base.object.class = vm->lib.String;
    str->base.object.properties = js_st_table_new();
    str->string.buff = string->buff;
    str->string.length = string->length;
    VAL v = js_value_make_pointer((js_value_t*)str);
    js_object_put(v, js_cstring("length"), js_value_make_double(str->string.length));
    return v;
}

static bool is_string_integer(js_string_t* str)
{
    uint32_t i;
    for(i = 0; i < str->length; i++) {
        if(str->buff[i] < '0' || str->buff[i] > '9') {
            // not an integer
            return false;
        }
    }
    return true;
}

static VAL string_vtable_get(js_value_t* obj, js_string_t* prop)
{
    js_string_object_t* str = (js_string_object_t*)obj;
    if(is_string_integer(prop)) {
        uint32_t idx = atoi(prop->buff);
        if(idx < str->string.length) {
            char x = str->string.buff[idx];
            return js_value_make_string(&x, 1);
        }
    }
    return js_object_base_vtable()->get(obj, prop);
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
        js_throw_error(vm->lib.TypeError, "String.prototype.toString() is not generic");
    }
    js_string_object_t* strobj = ((js_string_object_t*)js_value_get_pointer(this));
    return js_value_wrap_string(&strobj->string);
}

static VAL String_prototype_valueOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        js_throw_error(vm->lib.TypeError, "String.prototype.valueOf() is not generic");
    }
    js_string_object_t* strobj = ((js_string_object_t*)js_value_get_pointer(this));
    return js_value_wrap_string(&strobj->string);
}

static VAL String_prototype_substr(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        js_throw_error(vm->lib.TypeError, "String.prototype.substr() is not generic");
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
        js_throw_error(vm->lib.TypeError, "String.prototype.trimRight() is not generic");
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

static VAL String_prototype_indexOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.substr() is not generic");
    }
    if(argc > 0) {
        uint32_t index = 0;
        js_string_t* needle = js_to_js_string_t(argv[0]);
        js_string_t* haystack = js_to_js_string_t(this);
        if(js_string_index_of(haystack, needle, &index)) {
            return js_value_make_double(index);
        }
    }
    return js_value_make_double(-1);
}

static VAL String_prototype_split(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.substr() is not generic");
    }
    if(argc == 0) {
        return js_make_array(vm, 1, &this);
    }
    js_string_t* delimiter = js_to_js_string_t(argv[0]);
    uint32_t capacity = 4;
    uint32_t count = 0;
    VAL* items = js_alloc(sizeof(VAL) * 4);
    js_string_t remaining = ((js_string_object_t*)js_value_get_pointer(this))->string;
    uint32_t index;
    while(js_string_index_of(&remaining, delimiter, &index)) {
        if(count + 1 == capacity) {
            capacity *= 2;
            items = js_realloc(items, sizeof(VAL) * capacity);
        }
        items[count++] = js_value_make_string(remaining.buff, index);
        index += delimiter->length;
        remaining.buff += index;
        remaining.length -= index;
    }
    items[count++] = js_value_make_string(remaining.buff, remaining.length);
    return js_make_array(vm, count, items);
}

static VAL String_prototype_toLowerCase(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_STRING_OBJECT) {
        // @TODO throw exception
        js_panic("String.prototype.substr() is not generic");
    }
    js_string_object_t* str = (js_string_object_t*)js_value_get_pointer(this);
    js_value_t* new_str = (js_value_t*)js_alloc(sizeof(js_value_t));
    new_str->type = JS_T_STRING;
    new_str->string.buff = js_alloc_no_pointer(str->string.length + 1);
    new_str->string.length = str->string.length;
    memcpy(new_str->string.buff, str->string.buff, str->string.length);
    uint32_t i;
    for(i = 0; i < str->string.length; i++) {
        if(new_str->string.buff[i] >= 'A' && new_str->string.buff[i] <= 'Z') {
            new_str->string.buff[i] += 'a' - 'A';
        }
    }
    return js_value_make_pointer(new_str);
}

void js_lib_string_initialize(js_vm_t* vm)
{
    if(!statically_initialized) {
        statically_initialized = true;
        memcpy(&string_vtable, js_object_base_vtable(), sizeof(js_object_internal_methods_t));
        string_vtable.get = string_vtable_get;
    }
    
    vm->lib.String = js_value_make_native_function(vm, NULL, js_cstring("String"), String_call, String_construct);
    js_object_put(vm->global_scope->global_object, js_cstring("String"), vm->lib.String);
    
    vm->lib.String_prototype = js_value_make_object(vm->lib.Object_prototype, vm->lib.String);
    js_object_put(vm->lib.String, js_cstring("prototype"), vm->lib.String_prototype);
    js_object_put(vm->lib.String, js_cstring("fromCharCode"), js_value_make_native_function(vm, NULL, js_cstring("fromCharCode"), String_fromCharCode, NULL));
    
    js_object_put(vm->lib.String_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), String_prototype_toString, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("valueOf"), js_value_make_native_function(vm, NULL, js_cstring("valueOf"), String_prototype_valueOf, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("substr"), js_value_make_native_function(vm, NULL, js_cstring("substr"), String_prototype_substr, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("trimRight"), js_value_make_native_function(vm, NULL, js_cstring("trimRight"), String_prototype_trimRight, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("indexOf"), js_value_make_native_function(vm, NULL, js_cstring("indexOf"), String_prototype_indexOf, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("split"), js_value_make_native_function(vm, NULL, js_cstring("split"), String_prototype_split, NULL));
    js_object_put(vm->lib.String_prototype, js_cstring("toLowerCase"), js_value_make_native_function(vm, NULL, js_cstring("toLowerCase"), String_prototype_toLowerCase, NULL));
}