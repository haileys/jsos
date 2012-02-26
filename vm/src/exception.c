#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "exception.h"
#include "vm.h"
#include "gc.h"

// this is implemented as an accessor function to later allow for some thread safe stuff:
static bool initialized;
static js_exception_handler_t* handler;
js_exception_handler_t* js_current_exception_handler()
{
    return handler;
}
void js_set_exception_handler(js_exception_handler_t* exception_handler)
{
    if(!initialized) {
        initialized = true;
        js_gc_register_global(&handler, sizeof(js_exception_handler_t*));
    }
    handler = exception_handler;
}

static void(*js_panic_handler)(const char*, char*, int, char*);

void js_set_panic_handler(void(*panic_handler)(const char*, char*, int, char*))
{
    js_panic_handler = panic_handler;
}

void js_panic_impl(const char* func, char* file, int line, char* fmt, ...)
{
    char buff[2048];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buff, 2047, fmt, va);
    va_end(va);
    if(js_panic_handler) {
        js_panic_handler(func, file, line, buff);
    }
    printf("[PANIC] %s\n        in %s() at %s:%d\n", buff, func, file, line);
    exit(-1);
}

void js_throw(VAL exception)
{
    if(js_current_exception_handler() == NULL) {
        js_panic("exception thrown with no handler");
    }
    js_current_exception_handler()->exception = exception;
    longjmp(js_current_exception_handler()->env, 0);
}

void js_throw_message(js_vm_t* vm, char* message)
{
    js_throw(js_make_error(vm->lib.Error, js_cstring(message)));
}

bool js_try(void* state, void(*callback)(void*), VAL* exception)
{
    js_exception_handler_t* handler = js_alloc(sizeof(js_exception_handler_t));
    handler->previous = js_current_exception_handler();
    js_set_exception_handler(handler);
    handler->exception = js_value_undefined();
    if(setjmp(handler->env) == 0) {
        callback(state);
        js_set_exception_handler(handler->previous);
        return true;
    } else {
        /* exception was thrown */
        *exception = handler->exception;
        js_set_exception_handler(handler->previous);
        return false;
    }
}