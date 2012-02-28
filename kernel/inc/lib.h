#ifndef LIB_H
#define LIB_H

#include <vm.h>

void lib_vm_init(js_vm_t* vm);
void lib_kernel_init(js_vm_t* vm);
void lib_binary_utils_init(js_vm_t* vm);

#endif