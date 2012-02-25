#include <vm.h>
#include <value.h>
#include <exception.h>
#include "io.h"

static VAL js_inb(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.inb() expects port number parameter");
    }    
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    return js_value_make_double(inb(port));
}

void io_init(js_vm_t* vm)
{
    VAL Kernel = js_object_get(vm->global_scope->global_object, js_cstring("Kernel"));
    js_object_put(Kernel, js_cstring("inb"), js_value_make_native_function(vm, NULL, js_cstring("inb"), js_inb, NULL));
}