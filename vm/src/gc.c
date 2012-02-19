#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gc.h"
#include "exception.h"

static uint16_t pointer_hash(void* ptr)
{
    return (2654435761ul * (intptr_t)ptr) >> 16;
}

typedef struct allocation {
    struct allocation* next;
    struct allocation* prev;
    void* ptr;
    size_t size;
    #ifdef JS_GC_DEBUG
        char* file;
        int line;
    #endif
    bool flag;
} alloc_t;

typedef struct global {
    intptr_t** ptr;
    struct global* next;
} global_t;

static bool current_mark_flag;
static alloc_t* allocs[65536];
static global_t* globals;
static intptr_t* stack_top;
static size_t memory_usage;

static alloc_t* allocs_lookup(void* ptr)
{    
    uint16_t h = pointer_hash(ptr);
    alloc_t* st = allocs[h];
    while(st) {
        if(st->ptr == ptr) {
            return st;
        }
        st = st->next;
    }
    return NULL;
}

static alloc_t* allocs_insert(void* ptr, size_t size)
{
    uint16_t h = pointer_hash(ptr);
    alloc_t* alloc = malloc(sizeof(alloc_t));
    alloc->ptr = ptr;
    alloc->flag = current_mark_flag;
    alloc->size = size;
    alloc->next = allocs[h];
    if(alloc->next) {
        alloc->next->prev = alloc;
    }
    alloc->prev = NULL;
    allocs[h] = alloc;
    return alloc;
}

static void allocs_delete_alloc(alloc_t* alloc, uint16_t h)
{    
    if(alloc->prev == NULL) {
        allocs[h] = NULL;
    } else {
        alloc->prev->next = alloc->next;
    }
    if(alloc->next) {
        alloc->next->prev = alloc->prev;
    }
    #ifdef JS_GC_DEBUG
        memset(alloc->ptr, 0, alloc->size);
    #endif
    free(alloc->ptr);
    memory_usage -= alloc->size;
    free(alloc);
}

#ifdef JS_GC_DEBUG
void* js_alloc_impl(size_t sz, char* file, int line)
#else
void* js_alloc(size_t sz)
#endif
{
    void* ptr;
    alloc_t* alloc;
    if(stack_top == NULL) {
        js_panic("js_alloc() called before js_gc_init()");
    }
    ptr = malloc(sz);
    memory_usage += sz;
    if(ptr == NULL) {
        js_panic("malloc(%lu) failed!", sz);
    }
    alloc = allocs_insert(ptr, sz);
    #ifdef JS_GC_DEBUG
        alloc->file = file;
        alloc->line = line;
    #endif
    return ptr;
}

#ifdef JS_GC_DEBUG
void* js_realloc_impl(void* ptr, size_t sz, char* file, int line)
#else
void* js_realloc(void* ptr, size_t sz)
#endif
{
    #ifdef JS_GC_DEBUG
        void* new_ptr = js_alloc_impl(sz, file, line);
    #else
        void* new_ptr = js_alloc(sz);
    #endif
    alloc_t* alloc = allocs_lookup(ptr);
    if(alloc) {
        memcpy(new_ptr, ptr, sz > alloc->size ? alloc->size : sz);
    }
    return new_ptr;
    // @TODO attempt to realloc existing block
    /*
    alloc_t* alloc = allocs_lookup(ptr);
    void* new_ptr;
    uint16_t new_hash;
    if(alloc == NULL) {
        return js_alloc(sz);
    }
    new_ptr = realloc(ptr, sz);
    if(ptr == new_ptr) {
        return ptr;
    }
    alloc->ptr = new_ptr;
    alloc->size = sz;
    if(alloc->prev) {
        alloc->prev->next = alloc->next;
    }
    if(alloc->next) {
        alloc->next->prev = alloc->prev;
    }
    new_hash = pointer_hash(alloc->ptr);
    alloc->prev = NULL;
    alloc->next = allocs[new_hash];
    if(alloc->next) {
        alloc->next->prev = alloc;
    }
    allocs[new_hash] = alloc;
    return new_ptr;
    */
}

void js_gc_init(void* stack_ptr)
{
    // make sure the stack_ptr passed in is pointer aligned:
    stack_top = (void*)(((intptr_t)stack_ptr + sizeof(intptr_t)) & ~(sizeof(intptr_t) - 1));
}

void js_gc_register_global(void** address)
{
    global_t* g = malloc(sizeof(global_t));
    g->ptr = (intptr_t**)address;
    g->next = globals;
    globals = g;
}

size_t js_gc_memory_usage()
{
    return memory_usage;
}

static void js_gc_mark_allocation(alloc_t* alloc)
{
    intptr_t** ptrptr = alloc->ptr;
    alloc_t* suballoc;
    if(alloc->flag == current_mark_flag) {
        return;
    }
    alloc->flag = current_mark_flag;
    while((intptr_t)ptrptr < (intptr_t)((intptr_t)alloc->ptr + alloc->size)) {
        suballoc = allocs_lookup(*ptrptr);
        if(suballoc) {
            js_gc_mark_allocation(suballoc);
        }
        ptrptr++;
    }
}

static void js_gc_mark()
{
    uint32_t stack_dummy;
    intptr_t** ptrptr = (intptr_t**)stack_top;
    alloc_t* alloc;
    global_t* g;
    while((intptr_t)ptrptr > (intptr_t)&stack_dummy) {
        alloc = allocs_lookup(*ptrptr);
        if(alloc) {
            js_gc_mark_allocation(alloc);
        }
        ptrptr--;
    }
    for(g = globals; g; g = g->next) {
        alloc = allocs_lookup(*g->ptr);
        if(alloc) {
            js_gc_mark_allocation(alloc);
        }
    }
}

static void js_gc_sweep()
{
    uint32_t i;
    alloc_t* alloc;
    alloc_t* next;
    for(i = 0; i < 65536; i++) {
        alloc = allocs[i];
        while(alloc) {
            next = alloc->next;
            if(alloc->flag != current_mark_flag) {
                allocs_delete_alloc(alloc, pointer_hash(alloc->ptr));
            }
            alloc = next;
        }
    }
}

void js_gc_run()
{    
    current_mark_flag = !current_mark_flag;
    js_gc_mark();
    js_gc_sweep();
}