#include <value.h>
#include <vm.h>
#include <gc.h>
#include <exception.h>
#include "lib.h"

static VAL Kernel;

static VAL Kernel_load_image(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_STRING) {
        js_throw_message(vm, "Kernel.loadImage() expects string");
    }
    js_string_t* str = js_to_js_string_t(argv[0]);
    js_image_t* image = js_image_parse(str->buff, str->length);
    if(!image) {
        js_throw_message(vm, "Couldn't parse image");
    }
    js_vm_exec(vm, image, 0, vm->global_scope, js_value_null(), 0, NULL);
    return js_value_true();
}

static VAL Kernel_memory_usage(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_make_double(js_gc_memory_usage());
}

static VAL Kernel_run_gc(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_gc_run();
    return js_value_undefined();
}

void lib_kernel_init(js_vm_t* vm)
{
    Kernel = js_make_object(vm);
    js_gc_register_global(&Kernel, sizeof(Kernel));
    js_object_put(vm->global_scope->global_object, js_cstring("Kernel"), Kernel);
    
    js_object_put(Kernel, js_cstring("loadImage"), js_value_make_native_function(vm, NULL, js_cstring("loadImage"), Kernel_load_image, NULL));
    js_object_put(Kernel, js_cstring("memoryUsage"), js_value_make_native_function(vm, NULL, js_cstring("memoryUsage"), Kernel_memory_usage, NULL));
    js_object_put(Kernel, js_cstring("runGC"), js_value_make_native_function(vm, NULL, js_cstring("runGC"), Kernel_run_gc, NULL));
}