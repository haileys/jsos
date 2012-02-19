#ifndef LIBC_STDARG_H
#define LIBC_STDARG_H

#include "stdint.h"

typedef uint32_t* va_list;

#define va_start(list,argbefore) list = (void*)&(argbefore)
#define va_arg(list,type) (*(type*)(++(list)))
#define va_end(list) 

#endif