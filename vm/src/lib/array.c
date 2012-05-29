#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "lib.h"
#include "gc.h"
#include "object.h"
#include "exception.h"

typedef struct {
    js_value_t base;
    uint32_t length;
    uint32_t items_length;
    uint32_t capacity;
    VAL* items;
} js_array_t;

static bool statically_initialized;
static js_object_internal_methods_t array_vtable;

static VAL array_length_get(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_ARRAY) {
        js_throw_error(vm->lib.TypeError, "cannot find length of non array");
    }
    return js_value_make_double(((js_array_t*)js_value_get_pointer(this))->length);
}

VAL js_make_array(struct js_vm* vm, uint32_t count, VAL* items)
{
    js_array_t* ary = js_alloc(sizeof(js_array_t));
    ary->base.type = JS_T_ARRAY;
    ary->base.object.vtable = &array_vtable;
    ary->base.object.prototype = vm->lib.Array_prototype;
    ary->base.object.class = vm->lib.Array;
    ary->base.object.properties = js_st_table_new();
    ary->length = count;
    ary->items_length = count;
    ary->capacity = count < 4 ? 4 : count;
    ary->items = js_alloc(sizeof(VAL) * ary->capacity);
    memcpy(ary->items, items, sizeof(VAL) * ary->capacity);
    
    VAL obj = js_value_make_pointer((js_value_t*)ary);
    
    // length property
    js_object_put_accessor(vm, obj, "length", array_length_get, NULL);
    
    return obj;
}

VAL* js_array_items(VAL array, uint32_t* count)
{
    if(js_value_get_type(array) != JS_T_ARRAY) {
        js_panic("non array passed to js_array_items");
    }
    js_array_t* ary = (js_array_t*)js_value_get_pointer(array);
    VAL* out = js_alloc(sizeof(VAL) * ary->length);
    memcpy(out, ary->items, sizeof(VAL) * ary->items_length);
    uint32_t i;
    for(i = ary->items_length; i < ary->length; i++) {
        out[i] = js_value_undefined();
    }
    *count = ary->length;
    return out;
}

uint32_t js_array_length(VAL array)
{
    if(js_value_get_type(array) != JS_T_ARRAY) {
        js_panic("non array passed to js_array_length");
    }
    js_array_t* ary = (js_array_t*)js_value_get_pointer(array);
    return ary->length;
}

VAL js_array_get(VAL array, uint32_t idx)
{
    if(js_value_get_type(array) != JS_T_ARRAY) {
        js_panic("non array passed to js_array_get");
    }
    js_array_t* ary = (js_array_t*)js_value_get_pointer(array);
    if(idx < ary->items_length) {
        return ary->items[idx];
    }
    return js_value_undefined();
}

static void array_put(js_array_t* ary, uint32_t index, VAL val)
{
    if(index >= ary->length) {
        ary->length = index + 1;
    }
    if(index >= ary->capacity) {
        while(index >= ary->capacity) {
            ary->capacity *= 2;
        }
        ary->items = js_realloc(ary->items, sizeof(VAL) * ary->capacity);
    }
    while(index >= ary->items_length) {
        ary->items[ary->items_length++] = js_value_undefined();
    }
    ary->items[index] = val;
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

static VAL array_vtable_get(js_value_t* obj, js_string_t* prop)
{
    uint32_t index;
    js_array_t* ary = (js_array_t*)obj;
    if(is_string_integer(prop)) {
        index = atoi(prop->buff);
        if(index < ary->items_length) {
            return ary->items[index];
        }
        if(index >= ary->items_length && index < ary->length) {
            // sparse array
            return js_value_undefined();
        }
    }
    return js_object_base_vtable()->get(obj, prop);
}

static void array_vtable_put(js_value_t* obj, js_string_t* prop, VAL val)
{
    uint32_t index;
    js_array_t* ary = (js_array_t*)obj;
    if(is_string_integer(prop)) {
        index = atoi(prop->buff);
        array_put(ary, index, val);
    } else {
        js_object_base_vtable()->put(obj, prop, val);
    }
}

static bool array_vtable_has_property(js_value_t* obj, js_string_t* prop)
{
    uint32_t index;
    js_array_t* ary = (js_array_t*)obj;
    if(is_string_integer(prop)) {
        index = atoi(prop->buff);
        return index < ary->items_length;
    }
    return js_object_base_vtable()->has_property(obj, prop);
}

static bool array_vtable_delete(js_value_t* obj, js_string_t* prop)
{
    uint32_t index;
    js_array_t* ary = (js_array_t*)obj;
    if(is_string_integer(prop)) {
        index = atoi(prop->buff);
        if(index < ary->items_length) {
            ary->items[index] = js_value_undefined();
        }
        return true;
    }
    return js_object_base_vtable()->delete(obj, prop);
}

static js_string_t** array_vtable_keys(js_value_t* obj, uint32_t* out_count)
{
    uint32_t count, i;
    js_array_t* ary = (js_array_t*)obj;
    js_string_t** keys = js_object_base_vtable()->keys(obj, &count);
    keys = js_realloc(keys, sizeof(*keys) * (count + ary->items_length));
    for(i = 0; i < ary->items_length; i++) {
        keys[count + i] = js_string_format("%d", i);
    }
    *out_count = count + ary->items_length;
    return keys;
}

char* utoa(unsigned int value, char* buff, int base)
{
    char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* ret = buff;
    char scratch[64];
    int idx = 0;
    if(value == 0) {
        *buff++ = '0';
        *buff = 0;
        return ret;
    }
    while(value > 0) {
        scratch[idx++] = charset[value % base];
        value /= base;
    }
    while(idx > 0) {
        *buff++ = scratch[--idx];
    }
    *buff = 0;
    return ret;
}

static js_array_t* as_array(js_vm_t* vm, VAL val)
{
    if(js_value_get_type(val) == JS_T_ARRAY) {
        return (js_array_t*)js_value_get_pointer(val);
    }
    if(js_value_is_primitive(val)) {
        return as_array(vm, js_to_object(vm, val));
    }
    uint32_t i, length = js_to_uint32(js_object_get(val, js_cstring("length")));
    js_array_t* ary = (js_array_t*)js_value_get_pointer(js_make_array(vm, 0, NULL));
    char buff[16];
    for(i = 0; i < length; i++) {
        utoa(i, buff, 10);
        js_string_t* str = js_cstring(buff);
        if(js_object_has_property(val, str)) {
            array_put(ary, i, js_object_get(val, str));
        }
    }
    return ary;
}

static VAL Array_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_array_t* ary;
    uint32_t ary_length;
    if(argc == 1) {
        ary_length = js_to_uint32(argv[0]);
        ary = (js_array_t*)js_value_get_pointer(js_make_array(vm, ary_length, NULL));
        ary->length = ary_length;
        return js_value_make_pointer((js_value_t*)ary);
    } else {
        return js_make_array(vm, argc, argv);
    }
}

static VAL Array_prototype_push(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_ARRAY) {
        js_throw_error(vm->lib.TypeError, "Array.prototype.push is not generic");
    }
    js_array_t* ary = (js_array_t*)js_value_get_pointer(this);
    uint32_t i;
    for(i = 0; i < argc; i++) {
        array_put(ary, ary->length, argv[i]);
    }
    return js_value_make_double(ary->length);
}

static VAL Array_prototype_slice(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc && argv == 0) {
        js_panic("argv == NULL");
    }
    js_array_t* ary = as_array(vm, this);
    if(argc == 0) {
        return js_value_make_pointer((js_value_t*)ary);
    }
    js_array_t* new_ary = (js_array_t*)js_value_get_pointer(js_make_array(vm, 0, NULL));
    uint32_t begin = js_to_uint32(argv[0]);
    uint32_t end = ary->length;
    if(argc >= 2) {
        end = js_to_uint32(argv[1]);
        if(end > ary->length) {
            end = ary->length;
        }
    }
    if(ary->items_length >= begin) {
        new_ary->items_length = (end < ary->items_length ? end : ary->items_length) - begin;
        new_ary->capacity = new_ary->items_length;
        new_ary->items = js_alloc(sizeof(VAL) * new_ary->items_length);
        new_ary->length = end - begin;
        memcpy(new_ary->items, ary->items + begin, sizeof(VAL) * new_ary->items_length);
    } else if(end > begin) {
        new_ary->length = ary->length - begin;
    } else {
        new_ary->length = 0;
    }
    return js_value_make_pointer((js_value_t*)new_ary);
}

static VAL Array_prototype_splice(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_ARRAY) {
        js_throw_error(vm->lib.TypeError, "Array.prototype.splice is not generic");
    }
    js_array_t* ary = (js_array_t*)js_value_get_pointer(this);
    if(argc == 0) {
        return js_make_array(vm, 0, NULL);
    }
    
    uint32_t begin = js_to_uint32(argv[0]);
    uint32_t remove_length = argc > 1 ? js_to_uint32(argv[1]) : ary->length - begin;
    if(remove_length + begin > ary->length) {
        remove_length = ary->length - begin;
    }
    uint32_t replace_length = argc > 2 ? argc - 2 : 0;
    uint32_t trailer_begin = begin + remove_length;
    
    uint32_t new_length = ary->length - remove_length + replace_length;
    
    // i'm going to ignore outputting sparse arrays for now:
    uint32_t i;
    VAL* old_items = js_alloc(sizeof(VAL) * (remove_length > 4 ? remove_length : 4));
    for(i = 0; i < remove_length; i++) {
        uint32_t ary_i = begin + i;
        if(ary_i > ary->items_length) {
            old_items[i] = js_value_undefined();
        } else {
            old_items[i] = ary->items[ary_i];
        }
    }
    
    VAL* new_items = js_alloc(sizeof(VAL) * (new_length > 4 ? new_length : 4));
    uint32_t new_index = 0;
    for(i = 0; i < begin; i++) {
        if(i > ary->items_length) {
            new_items[new_index++] = js_value_undefined();
        } else {
            new_items[new_index++] = ary->items[i];
        }
    }
    for(i = 0; i < replace_length; i++) {
        new_items[new_index++] = argv[2 + i];
    }
    for(i = trailer_begin; i < ary->items_length; i++) {
        if(i > ary->items_length) {
            new_items[new_index++] = js_value_undefined();
        } else {
            new_items[new_index++] = ary->items[i];
        }
    }
    ary->items_length = new_length;
    ary->length = new_length;
    ary->capacity = new_length > 4 ? new_length : 4;
    ary->items = new_items;
    
    return js_make_array(vm, remove_length, old_items);
}

static VAL Array_prototype_join(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_array_t* ary = as_array(vm, this);
    if(ary->length == 0) {
        return js_value_make_cstring("");
    }
    js_string_t* joiner = argc > 0 ? js_to_js_string_t(argv[0]) : js_cstring(",");
    js_string_t* str = js_to_js_string_t(ary->items_length > 0 ? ary->items[0] : js_value_undefined());
    uint32_t i;
    for(i = 1; i < ary->length; i++) {
        str = js_string_concat(str, joiner);
        str = js_string_concat(str, js_to_js_string_t(ary->items_length > i ? ary->items[i] : js_value_undefined()));
    }
    return js_value_wrap_string(str);
}

static VAL Array_prototype_concat(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_array_t* ary = as_array(vm, this);
    uint32_t total_len = ary->length;
    uint32_t i;
    for(i = 0; i < argc; i++) {
        if(js_value_get_type(argv[i]) == JS_T_ARRAY) {
            total_len += ((js_array_t*)js_value_get_pointer(argv[i]))->length;
        } else {
            total_len++;
        }
    }
    VAL* vec = js_alloc(sizeof(VAL) * total_len);
    for(i = 0; i < ary->length; i++) {
        if(i < ary->items_length) {
            vec[i] = ary->items[i];
        } else {
            vec[i] = js_value_undefined();
        }
    }
    uint32_t j;
    for(j = 0; j < argc; j++) {
        if(js_value_get_type(argv[j]) == JS_T_ARRAY) {
            js_array_t* argary = (js_array_t*)js_value_get_pointer(argv[j]);
            uint32_t k;
            for(k = 0; k < argary->length; k++) {
                if(k < argary->items_length) {
                    vec[i++] = argary->items[k];
                } else {
                    vec[i++] = js_value_undefined();
                }
            }
        } else {
            vec[i++] = argv[j];
        }
    }
    return js_make_array(vm, total_len, vec);
}

static VAL Array_prototype_reduce(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_array_t* ary = as_array(vm, this);
    uint32_t i = 0;
    VAL callback = argc ? argv[0] : js_value_undefined();
    if(js_value_get_type(callback) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "First argument to Array.prototype.reduce must be a function");
    }
    if(ary->items_length == 0 && argc < 2) {
        js_throw_error(vm->lib.TypeError, "Reduce of empty array with no initial value");
    }
    VAL acc;
    if(argc > 1) {
        acc = argv[1];
    } else {
        acc = ary->items[0];
        i++;
    }
    for(; i < ary->items_length; i++) {
        VAL args[] = { acc, ary->items[i], js_value_make_double(i), js_value_make_pointer((js_value_t*)ary) };
        acc = js_call(callback, js_value_null(), 4, args);
    }
    return acc;
}

static VAL Array_prototype_reduceRight(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_array_t* ary = as_array(vm, this);
    uint32_t i = 0;
    VAL callback = argc ? argv[0] : js_value_undefined();
    if(js_value_get_type(callback) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "First argument to Array.prototype.reduce must be a function");
    }
    if(ary->items_length == 0 && argc < 2) {
        js_throw_error(vm->lib.TypeError, "Reduce of empty array with no initial value");
    }
    VAL acc;
    if(argc > 1) {
        acc = argv[1];
    } else {
        acc = ary->items[ary->items_length - 1];
        i++;
    }
    for(; i < ary->items_length; i++) {
        VAL args[] = { acc, ary->items[ary->items_length - i - 1], js_value_make_double(ary->items_length - i - 1), js_value_make_pointer((js_value_t*)ary) };
        acc = js_call(callback, js_value_null(), 4, args);
    }
    return acc;
}

void js_lib_array_initialize(js_vm_t* vm)
{
    if(!statically_initialized) {
        statically_initialized = true;
        memcpy(&array_vtable, js_object_base_vtable(), sizeof(js_object_internal_methods_t));
        array_vtable.get = array_vtable_get;
        array_vtable.put = array_vtable_put;
        array_vtable.has_property = array_vtable_has_property;
        array_vtable.delete = array_vtable_delete;
        array_vtable.keys = array_vtable_keys;
    }
    
    vm->lib.Array = js_value_make_native_function(vm, NULL, js_cstring("Array"), Array_call, Array_call);
    vm->lib.Array_prototype = js_object_get(vm->lib.Array, js_cstring("prototype"));
    js_object_put(vm->global_scope->global_object, js_cstring("Array"), vm->lib.Array);
    js_object_put(vm->lib.Array_prototype, js_cstring("push"), js_value_make_native_function(vm, NULL, js_cstring("push"), Array_prototype_push, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("slice"), js_value_make_native_function(vm, NULL, js_cstring("slice"), Array_prototype_slice, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("splice"), js_value_make_native_function(vm, NULL, js_cstring("splice"), Array_prototype_splice, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("join"), js_value_make_native_function(vm, NULL, js_cstring("join"), Array_prototype_join, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("concat"), js_value_make_native_function(vm, NULL, js_cstring("concat"), Array_prototype_concat, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("reduce"), js_value_make_native_function(vm, NULL, js_cstring("reduce"), Array_prototype_reduce, NULL));
    js_object_put(vm->lib.Array_prototype, js_cstring("reduceRight"), js_value_make_native_function(vm, NULL, js_cstring("reduceRight"), Array_prototype_reduceRight, NULL));
}
