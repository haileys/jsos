#include <value.h>
#include <vm.h>
#include <gc.h>
#include <exception.h>
#include <string.h>
#include "lib.h"
#include "console.h"

static VAL VM;
static VAL VM_prototype;

static js_vm_t* get_vm(js_vm_t* vm, VAL this)
{
    if(js_value_get_type(this) != JS_T_OBJECT
    || js_value_get_pointer(js_value_get_pointer(this)->object.class) != js_value_get_pointer(VM)
    || js_value_get_pointer(js_value_get_pointer(this)->object.prototype) != js_value_get_pointer(VM_prototype)) {
        js_throw_error(vm->lib.TypeError, "Can't call VM instance methods with non-VM as object");
    }
    return (js_vm_t*)js_value_get_pointer(this)->object.state;
}

static VAL wrap_vm(js_vm_t* vm)
{
    VAL obj = js_value_make_object(VM_prototype, VM);
    js_value_get_pointer(obj)->object.state = vm;
    return obj;
}

static VAL VM_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_vm_t* new_vm = js_vm_new();
    return wrap_vm(new_vm);
}

static VAL VM_self(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return wrap_vm(vm);
}

static VAL VM_prototype_id(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_make_double((uint32_t)get_vm(vm, this));
}

static VAL VM_prototype_globals(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return get_vm(vm, this)->global_scope->global_object;
}

static VAL VM_prototype_globals_set(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        js_throw_error(vm->lib.TypeError, "expected value as first argument");
    }
    return get_vm(vm, this)->global_scope->global_object = argv[0];
}

static VAL VM_prototype_execute(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || (js_value_get_type(argv[0]) != JS_T_STRING && js_value_get_type(argv[0]) != JS_T_FUNCTION)) {
        js_throw_error(vm->lib.TypeError, "VM.prototype.execute expects either a function or image data");
    }
    
    js_vm_t* target_vm = get_vm(vm, this);
    
    js_image_t* image;
    uint32_t section;
    if(js_value_get_type(argv[0]) == JS_T_STRING) {
        // load new image from string
        js_string_t* str = js_to_js_string_t(argv[0]);
        image = js_image_parse(str->buff, str->length);
        if(!image) {
            js_throw_message(vm, "Couldn't parse image");
        }
        section = 0;
    } else {
        // run section in existing image
        js_function_t* fn = (js_function_t*)js_value_get_pointer(argv[0]);
        if(fn->is_native) {
            js_throw_error(target_vm->lib.TypeError, "Can't create VM from native function");
        }
        image = fn->js.image;
        section = fn->js.section;
    }
    
    return js_vm_exec(target_vm, image, section, target_vm->global_scope, js_value_null(), 0, NULL);
}

static VAL VM_prototype_expose_function(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    // this is necessary to prevent user processes from gaining access to the Kernel Function.prototype
    
    js_vm_t* target_vm = get_vm(vm, this);
    
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "expected function as first parameter");
    }
    
    js_function_t* kernel_fn = (js_function_t*)js_value_get_pointer(argv[0]);
    js_function_t* user_fn = js_alloc(sizeof(js_function_t));
    memcpy(user_fn, kernel_fn, sizeof(js_function_t));
    user_fn->base.object.class = target_vm->lib.Function;
    user_fn->base.object.prototype = target_vm->lib.Function_prototype;
    return js_value_make_pointer((js_value_t*)user_fn);
}

static VAL VM_prototype_create_object(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_make_object(get_vm(vm, this));
}

static VAL VM_prototype_create_array(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_make_array(get_vm(vm, this), 0, NULL);
}

void lib_vm_init(js_vm_t* vm)
{
    VM = js_value_make_native_function(vm, NULL, js_cstring("VM"), NULL, VM_construct);
    js_gc_register_global(&VM, sizeof(VM));
    VM_prototype = js_object_get(VM, js_cstring("prototype"));
    js_gc_register_global(&VM_prototype, sizeof(VM_prototype));
    js_object_put(vm->global_scope->global_object, js_cstring("VM"), VM);
    
    // class properties:
    js_object_put_accessor(vm, VM, "self", VM_self, NULL);
    js_object_put_accessor(vm, VM_prototype, "id", VM_prototype_id, NULL);
    js_object_put_accessor(vm, VM_prototype, "globals", VM_prototype_globals, VM_prototype_globals_set);
    
    // instance methods:
    js_object_put(VM_prototype, js_cstring("execute"), js_value_make_native_function(vm, NULL, js_cstring("execute"), VM_prototype_execute, NULL));
    js_object_put(VM_prototype, js_cstring("exposeFunction"), js_value_make_native_function(vm, NULL, js_cstring("exposeFunction"), VM_prototype_expose_function, NULL));
    js_object_put(VM_prototype, js_cstring("createObject"), js_value_make_native_function(vm, NULL, js_cstring("createObject"), VM_prototype_create_object, NULL));
    js_object_put(VM_prototype, js_cstring("createArray"), js_value_make_native_function(vm, NULL, js_cstring("createArray"), VM_prototype_create_array, NULL));
}