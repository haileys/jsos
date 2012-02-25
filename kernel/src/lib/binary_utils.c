#include <value.h>
#include <vm.h>
#include <gc.h>
#include "lib.h"

static VAL BinaryUtils;

static VAL BinaryUtils_readU32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    uint32_t u32 = *(uint32_t*)&js_value_get_pointer(buff)->string.buff[offset];
    return js_value_make_double(u32);
}

void lib_binary_utils_init(js_vm_t* vm)
{
    BinaryUtils = js_make_object(vm);
    js_gc_register_global((void**)&BinaryUtils);
    js_object_put(vm->global_scope->global_object, js_cstring("BinaryUtils"), BinaryUtils);
    
    js_object_put(BinaryUtils, js_cstring("readU32"), js_value_make_native_function(vm, NULL, js_cstring("readU32"), BinaryUtils_readU32, NULL));
}