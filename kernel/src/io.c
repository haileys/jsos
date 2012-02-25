#include <vm.h>
#include <value.h>
#include <exception.h>
#include <gc.h>
#include "io.h"

static VAL js_inb(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.inb() expects port number parameter");
    }    
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    return js_value_make_double(inb(port));
}

static VAL js_outb(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_NUMBER || js_value_get_type(argv[1]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.outb() expects port number and byte parameters");
    }
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    uint8_t byte = (uint8_t)js_value_get_double(argv[1]);
    outb(port, byte);
    return js_value_undefined();
}

static VAL js_insw(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_NUMBER || js_value_get_type(argv[1]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.outb() expects port number and buffer size parameters");
    }
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    uint32_t size = (uint32_t)js_value_get_double(argv[1]);
    char* buff = js_alloc(size * 2 + 1);
    insw(port, buff, size);
    return js_value_make_string(buff, size * 2);
}

void io_init(js_vm_t* vm)
{
    VAL Kernel = js_object_get(vm->global_scope->global_object, js_cstring("Kernel"));
    js_object_put(Kernel, js_cstring("inb"), js_value_make_native_function(vm, NULL, js_cstring("inb"), js_inb, NULL));
    js_object_put(Kernel, js_cstring("outb"), js_value_make_native_function(vm, NULL, js_cstring("outb"), js_outb, NULL));
    js_object_put(Kernel, js_cstring("insw"), js_value_make_native_function(vm, NULL, js_cstring("insw"), js_insw, NULL));
}