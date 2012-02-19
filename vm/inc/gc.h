#ifndef JS_GC_H
#define JS_GC_H

#include <stdint.h>
#include <stdlib.h>

#ifdef JS_GC_DEBUG
    void* js_alloc_impl(size_t sz, char* file, int line);
    void* js_realloc_impl(void* ptr, size_t sz, char* file, int line);
    
    #define js_alloc(sz) js_alloc_impl(sz, __FILE__, __LINE__)
    #define js_realloc(ptr, sz) js_realloc_impl(ptr, sz, __FILE__, __LINE__)
#else
    void* js_alloc(size_t sz);
    void* js_realloc(void* ptr, size_t sz);
#endif
void js_gc_init(void* stack_ptr);
void js_gc_register_global(void** address);
void js_gc_run();
size_t js_gc_memory_usage();

#endif