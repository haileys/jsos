#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gc.h"
#include "exception.h"

#define ALLOC_MAX_BUCKETS (65536 + 45)

static uint16_t pointer_hash(void* ptr)
{
    return (2654435761ul * (intptr_t)ptr) % ALLOC_MAX_BUCKETS;
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
    bool no_pointer;
} alloc_t;

typedef struct global {
    intptr_t** ptr;
    size_t size;
    struct global* next;
} global_t;

static bool current_mark_flag;
static alloc_t* allocs[ALLOC_MAX_BUCKETS];

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
    alloc->no_pointer = false;
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
        allocs[h] = alloc->next;
    } else {
        alloc->prev->next = alloc->next;
    }
    if(alloc->next) {
        alloc->next->prev = alloc->prev;
    }
    memset(alloc->ptr, 0, alloc->size);
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
    if(ptr == NULL) {
        // allocation failed, so run a gc and attempt to free up some space
        js_gc_run();
        ptr = malloc(sz);
        if(ptr == NULL) {
            js_panic("malloc(%u) failed - out of memory!", sz);
        }
    }
    memory_usage += sz;
    memset(ptr, 0, sz);
    alloc = allocs_insert(ptr, sz);
    #ifdef JS_GC_DEBUG
        alloc->file = file;
        alloc->line = line;
    #else
        (void)alloc;
    #endif
    return ptr;
}

#ifdef JS_GC_DEBUG
void* js_alloc_no_pointer_impl(size_t sz, char* file, int line)
#else
void* js_alloc_no_pointer(size_t sz)
#endif
{
    #ifdef JS_GC_DEBUG
        void* ptr = js_alloc_impl(sz, file, line);
    #else
        void* ptr = js_alloc(sz);
    #endif
    alloc_t* alloc = allocs_lookup(ptr);
    alloc->no_pointer = true;
    return ptr;    
}

#ifdef JS_GC_DEBUG
void* js_realloc_impl(void* ptr, size_t sz, char* file, int line)
#else
void* js_realloc(void* ptr, size_t sz)
#endif
{
    alloc_t* alloc = allocs_lookup(ptr);
    void* new_ptr;
    uint16_t new_hash;
    if(alloc == NULL) {
        #ifdef JS_GC_DEBUG
            return js_alloc_impl(sz, file, line);
        #else
            return js_alloc(sz);
        #endif
    }
    new_ptr = realloc(ptr, sz);
    if(new_ptr == ptr) {
        alloc->size = sz;
        return new_ptr;
    }
    
    // pointer has changed, so unlink this alloc from its bucket:    
    new_hash = pointer_hash(new_ptr);
    if(alloc->prev == NULL) {
        allocs[pointer_hash(ptr)] = alloc->next;
    } else {
        alloc->prev->next = alloc->next;
    }
    if(alloc->next) {
        alloc->next->prev = alloc->prev;
    }
    // change the pointer and size:
    alloc->ptr = new_ptr;
    alloc->size = sz;
    // add to the start of its new bucket:
    alloc->next = allocs[new_hash];
    if(alloc->next) {
        alloc->next->prev = alloc;
    }
    alloc->prev = NULL;
    allocs[new_hash] = alloc;
    // and return the pointer to the new allocation:
    return new_ptr;
}

void js_gc_init(void* stack_ptr)
{
    // make sure the stack_ptr passed in is pointer aligned:
    stack_top = (void*)(((intptr_t)stack_ptr + sizeof(intptr_t)) & ~(sizeof(intptr_t) - 1));
}

void js_gc_register_global(void* address, size_t length)
{
    global_t* g = malloc(sizeof(global_t));
    g->ptr = (intptr_t**)address;
    g->size = length / sizeof(intptr_t);
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
    if(alloc->no_pointer) {
        return;
    }
    while((intptr_t)ptrptr < (intptr_t)((intptr_t)alloc->ptr + alloc->size)) {
        suballoc = allocs_lookup(*ptrptr);
        if(suballoc) {
            js_gc_mark_allocation(suballoc);
        }
        ptrptr++;
    }
}

#ifdef __GNUC__
    #define NOINLINE __attribute__((noinline))
#else
    #define NOINLINE
#endif

NOINLINE static void js_gc_mark()
{
    uint32_t stack_dummy;
    intptr_t** ptrptr = (intptr_t**)stack_top;
    alloc_t* alloc;
    global_t* g;
    while((intptr_t)ptrptr > (intptr_t)&stack_dummy) {
        if(((intptr_t)*ptrptr & 3) == 0) {
            intptr_t p = *ptrptr;
            if(sizeof(intptr_t) == 8) {
                p &= 0x7ffffffffffful;
            }
            alloc = allocs_lookup(p);
            if(alloc) {
                js_gc_mark_allocation(alloc);
            }
        }
        ptrptr--;
    }
    for(g = globals; g; g = g->next) {
        uint32_t i;
        for(i = 0; i < g->size; i++) {
            alloc = allocs_lookup(g->ptr[i]);
            if(alloc) {
                js_gc_mark_allocation(alloc);
            }
        }
    }
}

NOINLINE static void js_gc_sweep()
{
    uint32_t i;
    alloc_t* alloc;
    alloc_t* next;
    for(i = 0; i < ALLOC_MAX_BUCKETS; i++) {
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
    #ifdef JSOS
        uint16_t* vram = (uint16_t*)0xb8000;
        uint16_t indicator = vram[79];
        vram[79] = ' ' | (5 << 12);
    #endif
    current_mark_flag = !current_mark_flag;
    js_gc_mark();
    js_gc_sweep();
    #ifdef JSOS
        vram[79] = indicator;
    #endif
}