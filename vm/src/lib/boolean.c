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
    bool boolean;
} js_boolean_object_t;

static VAL Boolean_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_value_false();
    } else {
        return js_to_boolean(argv[0]);
    }
}

static VAL Boolean_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_make_boolean_object(vm, false);
    } else {
        return js_make_boolean_object(vm, js_value_is_truthy(argv[0]));
    }
}

VAL js_make_boolean_object(js_vm_t* vm, bool boolean)
{
    js_boolean_object_t* obj = js_alloc(sizeof(js_boolean_object_t));
    obj->base.type = JS_T_BOOLEAN_OBJECT;
    obj->base.object.vtable = js_object_base_vtable();
    obj->base.object.prototype = vm->lib.Boolean_prototype;
    obj->base.object.class = vm->lib.Boolean;
    obj->base.object.properties = js_st_table_new();
    obj->boolean = boolean;
    return js_value_make_pointer((js_value_t*)obj);
}

static VAL Boolean_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_BOOLEAN_OBJECT) {
        js_throw_error(vm->lib.TypeError, "Boolean.prototype.toString() is not generic");
    }
    js_boolean_object_t* obj = (js_boolean_object_t*)js_value_get_pointer(this);
    return js_value_make_cstring(obj->boolean ? "true" : "false");
}

static VAL Boolean_prototype_valueOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_BOOLEAN_OBJECT) {
        js_throw_error(vm->lib.TypeError, "Boolean.prototype.valueOf() is not generic");
    }
    js_boolean_object_t* obj = (js_boolean_object_t*)js_value_get_pointer(this);
    return js_value_make_boolean(obj->boolean);
}

void js_lib_boolean_initialize(js_vm_t* vm)
{
    vm->lib.Boolean = js_value_make_native_function(vm, NULL, js_cstring("Boolean"), Boolean_call, Boolean_construct);
    js_object_put(vm->global_scope->global_object, js_cstring("Boolean"), vm->lib.Boolean);
    
    vm->lib.Boolean_prototype = js_value_make_object(vm->lib.Object_prototype, vm->lib.Boolean);
    js_object_put(vm->lib.Boolean, js_cstring("prototype"), vm->lib.Boolean_prototype);
    
    js_object_put(vm->lib.Boolean_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), Boolean_prototype_toString, NULL));
    js_object_put(vm->lib.Boolean_prototype, js_cstring("valueOf"), js_value_make_native_function(vm, NULL, js_cstring("valueOf"), Boolean_prototype_valueOf, NULL));
}