#ifndef JS_EXCEPTION_H
#define JS_EXCEPTION_H

#include <stddef.h>
#include <setjmp.h>
#include "value.h"

typedef struct exception_handler {
    struct exception_handler* previous;
    jmp_buf env;
    VAL exception;
} js_exception_handler_t;

void js_panic_impl(const char* func, char* file, int line, char* fmt, ...)
#ifdef __GNUC__
    __attribute__ ((noreturn))
#endif
;
#define js_panic(...) js_panic_impl(__func__, __FILE__, __LINE__, __VA_ARGS__)

js_exception_handler_t* js_current_exception_handler();
void js_set_exception_handler(js_exception_handler_t* exception_handler);

void js_throw(VAL exception)
#ifdef __GNUC__
    __attribute__ ((noreturn))
#endif
;
void js_throw_message(struct js_vm* vm, char* message);
bool js_try(void* state, void(*callback)(void*), VAL* exception);

#define JS_TRY(try_block, ex_var, catch_block) do { \
            js_exception_handler_t* __handler = js_alloc(sizeof(js_exception_handler_t)); \
            __handler->previous = js_current_exception_handler(); \
            js_set_exception_handler(__handler); \
            if(!setjmp(__handler->env)) { \
                try_block \
                js_set_exception_handler(__handler->previous); \
            } else { \
                ex_var = __handler->exception; \
                js_set_exception_handler(__handler->previous); \
                catch_block \
            } \
        } while(0)

#endif