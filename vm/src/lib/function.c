#include <stdio.h>
#include "lib.h"
#include "gc.h"
#include "exception.h"
#include "string.h"

static VAL Function_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    // @TODO: throw error
    js_panic("new functions may not be created at runtime\n");
    exit(-1);
    return js_value_null();
}

static VAL Empty_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_undefined();
}

static VAL Function_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_wrap_string(js_cstring("function() { ... }"));
}

void js_lib_function_initialize(struct js_vm* vm)
{
    vm->lib.Function = js_value_make_native_function(vm, NULL, js_cstring("Function"), Function_call, Function_call);
    vm->lib.Function_prototype = js_value_make_object(
        js_value_undefined() /* this will be fixed up after initialization */,
        js_value_undefined() /* this gets fixed up here */
    );
    js_object_put(vm->lib.Function, js_cstring("prototype"), vm->lib.Function_prototype);
    VAL Empty = js_value_make_native_function(vm, NULL, js_cstring("Empty"), Empty_call, NULL);
    js_value_get_pointer(vm->lib.Function_prototype)->object.class = Empty;
    js_object_put(vm->global_scope->global_object, js_cstring("Function"), vm->lib.Function);
    
    js_object_put(vm->lib.Function_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), Function_prototype_toString, NULL));
}