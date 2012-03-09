#include <stddef.h>
#include <math.h>
#include "lib.h"

static VAL Math_floor(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    double d = js_value_get_double(js_to_number(argc ? argv[0] : js_value_undefined()));
    return js_value_make_double(floor(d));
}

void js_lib_math_initialize(js_vm_t* vm)
{
    VAL Math = js_make_object(vm);
    js_object_put(vm->global_scope->global_object, js_cstring("Math"), Math);
    js_object_put(Math, js_cstring("floor"), js_value_make_native_function(vm, NULL, js_cstring("floor"), Math_floor, NULL));
}