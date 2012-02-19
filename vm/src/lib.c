#include "lib.h"

void js_lib_initialize(js_vm_t* vm)
{
    js_lib_function_initialize(vm);
    js_lib_object_initialize(vm);
    
    /* fix up function <-> object dependence: */
    js_value_get_pointer(vm->lib.Function)->object.prototype = vm->lib.Object_prototype;
    
    /* fix up global object */
    js_value_get_pointer(vm->global_scope->global_object)->object.class = vm->lib.Object;
    js_value_get_pointer(vm->global_scope->global_object)->object.prototype = vm->lib.Object_prototype;
    
    js_lib_error_initialize(vm);
    js_lib_array_initialize(vm);
    js_lib_number_initialize(vm);
}