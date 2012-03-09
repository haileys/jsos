#include <stddef.h>
#include <math.h>
#include "lib.h"

static VAL Math_floor(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    double d = js_value_get_double(js_to_number(argc ? argv[0] : js_value_undefined()));
    return js_value_make_double(floor(d));
}

static VAL Math_sin(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    double d = js_value_get_double(js_to_number(argc ? argv[0] : js_value_undefined()));
    return js_value_make_double(sin(d));
}

static VAL Math_cos(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    double d = js_value_get_double(js_to_number(argc ? argv[0] : js_value_undefined()));
    return js_value_make_double(cos(d));
}

static VAL Math_sqrt(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    double d = js_value_get_double(js_to_number(argc ? argv[0] : js_value_undefined()));
    return js_value_make_double(sqrt(d));
}

void js_lib_math_initialize(js_vm_t* vm)
{
    VAL Math = js_make_object(vm);
    js_object_put(vm->global_scope->global_object, js_cstring("Math"), Math);
    js_object_put(Math, js_cstring("floor"), js_value_make_native_function(vm, NULL, js_cstring("floor"), Math_floor, NULL));
    js_object_put(Math, js_cstring("cos"), js_value_make_native_function(vm, NULL, js_cstring("cos"), Math_cos, NULL));
    js_object_put(Math, js_cstring("sin"), js_value_make_native_function(vm, NULL, js_cstring("sin"), Math_sin, NULL));
    js_object_put(Math, js_cstring("sqrt"), js_value_make_native_function(vm, NULL, js_cstring("sqrt"), Math_sqrt, NULL));
    js_object_put(Math, js_cstring("PI"), js_value_make_double(M_PI));
}