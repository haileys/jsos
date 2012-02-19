#include "string.h"
#include "lib.h"
#include "gc.h"

static VAL Object_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_make_object(vm);
    } else {
        return js_to_object(vm, argv[0]);
    }
}

VAL js_make_object(struct js_vm* vm)
{    
    return js_value_make_object(vm->lib.Object_prototype, vm->lib.Object);
}

static VAL Object_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL class = js_value_get_pointer(this)->object.class;
    js_string_t* name = ((js_function_t*)js_value_get_pointer(class))->name;
    if(name && name->length) {
        return js_value_wrap_string(js_string_format("[object %s]", name->buff));
    } else {
        return js_value_wrap_string(js_cstring("[object]"));
    }
}

static VAL Object_prototype_valueOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return this;
}

void js_lib_object_initialize(struct js_vm* vm)
{
    vm->lib.Object = js_value_make_native_function(vm, NULL, js_cstring("Object"), Object_call, Object_call);
    vm->lib.Object_prototype = js_value_make_object(js_value_null(), vm->lib.Object);
    js_object_put(vm->lib.Object, js_cstring("prototype"), vm->lib.Object_prototype);
    js_object_put(vm->global_scope->global_object, js_cstring("Object"), vm->lib.Object);
    
    js_object_put(vm->lib.Object_prototype, js_cstring("valueOf"), js_value_make_native_function(vm, NULL, js_cstring("valueOf"), Object_prototype_valueOf, NULL));
    js_object_put(vm->lib.Object_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), Object_prototype_toString, NULL));
}