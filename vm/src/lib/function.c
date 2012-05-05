#include <stdio.h>
#include <string.h>
#include "lib.h"
#include "gc.h"
#include "exception.h"
#include "string.h"

static VAL Function_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    // @TODO: throw error
    js_panic("new functions may not be created at runtime\n");
}

static VAL Empty_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_undefined();
}

static VAL Function_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_wrap_string(js_cstring("function() { ... }"));
}

static VAL Function_prototype_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "object is not a function");
    }
    if(argc == 0) {
        return js_call(this, vm->global_scope->global_object, 0, NULL);
    } else {
        VAL* new_args = NULL;
        if(argc >= 2) {
            new_args = js_alloc(sizeof(VAL) * (argc - 1));
            memcpy(new_args, argv + 1, sizeof(VAL) * (argc - 1));
        }
        return js_call(this, argv[0], argc - 1, new_args);
    }
}

static VAL Function_prototype_apply(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "object is not a function");
    }
    if(argc == 0) {
        return js_call(this, vm->global_scope->global_object, 0, NULL);
    } else if(argc == 1) {
        return Function_prototype_call(vm, state, this, argc, argv);
    } else {
        if(js_value_get_type(argv[1]) != JS_T_ARRAY) {
            js_throw_error(vm->lib.TypeError, "expected array as second parameter to Function.prototype.apply");
        }
        uint32_t new_argc;
        VAL* new_args = js_array_items(argv[1], &new_argc);
        return js_call(this, argv[0], new_argc, new_args);
    }
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
    js_object_put(vm->lib.Function_prototype, js_cstring("call"), js_value_make_native_function(vm, NULL, js_cstring("call"), Function_prototype_call, NULL));
    js_object_put(vm->lib.Function_prototype, js_cstring("apply"), js_value_make_native_function(vm, NULL, js_cstring("apply"), Function_prototype_apply, NULL));
}