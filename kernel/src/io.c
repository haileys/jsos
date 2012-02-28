#include <vm.h>
#include <value.h>
#include <exception.h>
#include <gc.h>
#include <string.h>
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
    if(argc < 2 || js_value_get_type(argv[0]) != JS_T_NUMBER || js_value_get_type(argv[1]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.outb() expects port number and buffer size parameters");
    }
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    uint32_t size = (uint32_t)js_value_get_double(argv[1]);
    char* buff = js_alloc_no_pointer(size * 2 + 1);
    insw(port, buff, size);
    return js_value_make_string(buff, size * 2);
}

static VAL js_outsw(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc < 3 || js_value_get_type(argv[0]) != JS_T_NUMBER || js_value_get_type(argv[1]) != JS_T_STRING || js_value_get_type(argv[2]) != JS_T_NUMBER) {
        js_throw_message(vm, "Kernel.outb() expects port number, buffer and buffer size parameters");
    }
    uint16_t port = (uint16_t)js_value_get_double(argv[0]);
    uint32_t size = (uint32_t)js_value_get_double(argv[2]);
    char buff[size];
    memcpy(buff, js_value_get_pointer(argv[1])->string.buff, size);
    outsw(port, buff, size);
    return js_value_undefined();
}

void io_init(js_vm_t* vm)
{
    VAL Kernel = js_object_get(vm->global_scope->global_object, js_cstring("Kernel"));
    js_object_put(Kernel, js_cstring("inb"), js_value_make_native_function(vm, NULL, js_cstring("inb"), js_inb, NULL));
    js_object_put(Kernel, js_cstring("outb"), js_value_make_native_function(vm, NULL, js_cstring("outb"), js_outb, NULL));
    js_object_put(Kernel, js_cstring("insw"), js_value_make_native_function(vm, NULL, js_cstring("insw"), js_insw, NULL));
    js_object_put(Kernel, js_cstring("outsw"), js_value_make_native_function(vm, NULL, js_cstring("outsw"), js_outsw, NULL));
}