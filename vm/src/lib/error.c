#include <stddef.h>
#include "string.h"
#include "lib.h"
#include "exception.h"

static VAL make_generic_error(js_string_t* name, VAL proto, VAL class, uint32_t argc, VAL* argv)
{
    VAL obj = js_value_make_object(proto, class);
    js_object_put(obj, js_cstring("name"), js_value_wrap_string(name));
    if(argc > 0) {
        js_object_put(obj, js_cstring("message"), js_to_string(argv[0]));
    }
    return obj;
}

static VAL Error_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return make_generic_error(js_cstring("Error"), vm->lib.Error_prototype, vm->lib.Error, argc, argv);
}

static VAL RangeError_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return make_generic_error(js_cstring("RangeError"), vm->lib.RangeError_prototype, vm->lib.RangeError, argc, argv);
}

static VAL ReferenceError_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return make_generic_error(js_cstring("ReferenceError"), vm->lib.ReferenceError_prototype, vm->lib.ReferenceError, argc, argv);
}

static VAL TypeError_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return make_generic_error(js_cstring("TypeError"), vm->lib.TypeError_prototype, vm->lib.TypeError, argc, argv);
}

static VAL Error_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_string_t* message = &js_value_get_pointer(js_to_string(js_object_get(this, js_cstring("message"))))->string;
    js_string_t* name = &js_value_get_pointer(js_to_string(js_object_get(this, js_cstring("name"))))->string;
    js_string_t* str = js_string_concat(js_string_concat(name, js_cstring(": ")), message);
    return js_value_wrap_string(str);
}

VAL js_make_error(VAL class, js_string_t* message)
{
    VAL msg = js_value_wrap_string(message);
    return js_construct(class, 1, &msg);
}

void js_throw_error(VAL class, char* fmt, ...)
{
    va_list va;
    js_string_t* message;
    va_start(va, fmt);
    message = js_string_vformat(fmt, va);
    va_end(va);
    js_throw(js_make_error(class, message));
}

void js_lib_error_initialize(struct js_vm* vm)
{
    vm->lib.Error = js_value_make_native_function(vm, NULL, js_cstring("Error"), Error_construct, Error_construct);
    js_object_put(vm->global_scope->global_object, js_cstring("Error"), vm->lib.Error);
    vm->lib.Error_prototype = js_value_make_object(vm->lib.Object_prototype, vm->lib.Error);
    js_object_put(vm->lib.Error, js_cstring("prototype"), vm->lib.Error_prototype);
    js_object_put(vm->lib.Error_prototype, js_cstring("toString"), js_value_make_native_function(vm, js_cstring("Error"), js_cstring("toString"), Error_toString, NULL));
    
    vm->lib.RangeError = js_value_make_native_function(vm, NULL, js_cstring("RangeError"), RangeError_construct, RangeError_construct);
    vm->lib.RangeError_prototype = js_value_make_object(vm->lib.Error_prototype, vm->lib.Error);
    js_object_put(vm->lib.RangeError, js_cstring("prototype"), vm->lib.RangeError_prototype);
    js_object_put(vm->global_scope->global_object, js_cstring("RangeError"), vm->lib.RangeError);
    
    vm->lib.ReferenceError = js_value_make_native_function(vm, NULL, js_cstring("ReferenceError"), ReferenceError_construct, ReferenceError_construct);
    vm->lib.ReferenceError_prototype = js_value_make_object(vm->lib.Error_prototype, vm->lib.Error);
    js_object_put(vm->lib.ReferenceError, js_cstring("prototype"), vm->lib.ReferenceError_prototype);
    js_object_put(vm->global_scope->global_object, js_cstring("ReferenceError"), vm->lib.ReferenceError);
    
    vm->lib.TypeError = js_value_make_native_function(vm, NULL, js_cstring("TypeError"), TypeError_construct, TypeError_construct);
    vm->lib.TypeError_prototype = js_value_make_object(vm->lib.Error_prototype, vm->lib.Error);
    js_object_put(vm->lib.TypeError, js_cstring("prototype"), vm->lib.TypeError_prototype);
    js_object_put(vm->global_scope->global_object, js_cstring("TypeError"), vm->lib.TypeError);
}