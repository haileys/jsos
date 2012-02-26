#include <value.h>
#include <vm.h>
#include <gc.h>
#include "lib.h"

static VAL BinaryUtils;

static VAL BinaryUtils_readU64(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 8 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    uint64_t u32 = *(uint64_t*)&str->buff[offset];
    return js_value_make_double(u32);
}

static VAL BinaryUtils_readS64(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 8 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    int64_t s32 = *(int64_t*)&str->buff[offset];
    return js_value_make_double(s32);
}

static VAL BinaryUtils_readU32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 4 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    uint32_t u32 = *(uint32_t*)&str->buff[offset];
    return js_value_make_double(u32);
}

static VAL BinaryUtils_readS32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 4 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    int32_t s32 = *(int32_t*)&str->buff[offset];
    return js_value_make_double(s32);
}

static VAL BinaryUtils_readU16(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 2 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    uint16_t u16 = *(uint16_t*)&str->buff[offset];
    return js_value_make_double(u16);
}

static VAL BinaryUtils_readS16(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    VAL buff;
    uint32_t offset;
    js_scan_args(vm, argc, argv, "SI", &buff, &offset);
    js_string_t* str = &js_value_get_pointer(buff)->string;
    if(offset + 2 > str->length) {
        js_throw_error(vm->lib.RangeError, "tried to read past end of buffer of length %d (offset was %d)", str->length, offset);
    }
    int16_t s16 = *(int16_t*)&str->buff[offset];
    return js_value_make_double(s16);
}

void lib_binary_utils_init(js_vm_t* vm)
{
    BinaryUtils = js_make_object(vm);
    js_gc_register_global(&BinaryUtils, sizeof(BinaryUtils));
    js_object_put(vm->global_scope->global_object, js_cstring("BinaryUtils"), BinaryUtils);
    
    js_object_put(BinaryUtils, js_cstring("readU64"), js_value_make_native_function(vm, NULL, js_cstring("readU64"), BinaryUtils_readU64, NULL));
    js_object_put(BinaryUtils, js_cstring("readS64"), js_value_make_native_function(vm, NULL, js_cstring("readS64"), BinaryUtils_readS64, NULL));
    js_object_put(BinaryUtils, js_cstring("readU32"), js_value_make_native_function(vm, NULL, js_cstring("readU32"), BinaryUtils_readU32, NULL));
    js_object_put(BinaryUtils, js_cstring("readS32"), js_value_make_native_function(vm, NULL, js_cstring("readS32"), BinaryUtils_readS32, NULL));
    js_object_put(BinaryUtils, js_cstring("readU16"), js_value_make_native_function(vm, NULL, js_cstring("readU16"), BinaryUtils_readU16, NULL));
    js_object_put(BinaryUtils, js_cstring("readS16"), js_value_make_native_function(vm, NULL, js_cstring("readS16"), BinaryUtils_readS16, NULL));
}