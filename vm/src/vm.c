#include <stdlib.h>
#include <setjmp.h>
#include <math.h>
#include <string.h>
#include "vm.h"
#include "gc.h"
#include "object.h"
#include "lib.h"
#include "string.h"
#include "exception.h"

static js_instruction_t insns[] = {
    { "undefined",  OPERAND_NONE },
    { "ret",        OPERAND_NONE },
    { "pushnum",    OPERAND_NUMBER },
    { "add",        OPERAND_NONE },
    { "pushglobal", OPERAND_STRING },
    { "pushstr",    OPERAND_STRING },
    { "methcall",   OPERAND_UINT32 },
    { "setvar",     OPERAND_UINT32_UINT32 },
    { "pushvar",    OPERAND_UINT32_UINT32 },
    { "true",       OPERAND_NONE },
    { "false",      OPERAND_NONE },
    { "null",       OPERAND_NONE },
    { "jmp",        OPERAND_UINT32 },
    { "jit",        OPERAND_UINT32 },
    { "jif",        OPERAND_UINT32 },
    { "sub",        OPERAND_NONE },
    { "mul",        OPERAND_NONE },
    { "div",        OPERAND_NONE },
    { "setglobal",  OPERAND_STRING },
    { "close",      OPERAND_UINT32 },
    { "call",       OPERAND_UINT32 },
    { "setcallee",  OPERAND_UINT32 },
    { "setarg",     OPERAND_UINT32_UINT32 },
    { "lt",         OPERAND_NONE },
    { "lte",        OPERAND_NONE },
    { "gt",         OPERAND_NONE },
    { "gte",        OPERAND_NONE },
    { "pop",        OPERAND_NONE },
    { "array",      OPERAND_UINT32 },
    { "newcall",    OPERAND_UINT32 },
    { "throw",      OPERAND_NONE },
    { "member",     OPERAND_STRING },
    { "dup",        OPERAND_NONE },
    { "this",       OPERAND_NONE },
    { "setprop",    OPERAND_STRING },
    { "tst",        OPERAND_NONE },
    { "tld",        OPERAND_NONE },
    { "index",      OPERAND_NONE },
    { "setindex",   OPERAND_NONE },
    { "object",     OPERAND_UINT32 },
    { "typeof",     OPERAND_NONE },
    { "seq",        OPERAND_NONE },
    { "typeofg",    OPERAND_STRING },
    { "sal",        OPERAND_NONE },
    { "or",         OPERAND_NONE },
    { "xor",        OPERAND_NONE },
    { "and",        OPERAND_NONE },
    { "slr",        OPERAND_NONE },
    { "not",        OPERAND_NONE },
    { "bitnot",     OPERAND_NONE },
    { "line",       OPERAND_UINT32 },
    { "debugger",   OPERAND_NONE },
    { "instanceof", OPERAND_NONE },
    { "negate",     OPERAND_NONE },
    { "try",        OPERAND_UINT32_UINT32 },
    { "poptry",     OPERAND_NONE },
    { "catch",      OPERAND_UINT32 },
    { "catchg",     OPERAND_STRING },
    { "popcatch",   OPERAND_NONE },
    { "finally",    OPERAND_NONE },
    { "popfinally", OPERAND_NONE },
    { "closenamed", OPERAND_UINT32_STRING },
    { "delete",     OPERAND_NONE },
    { "mod",        OPERAND_NONE },
    { "arguments",  OPERAND_UINT32 },
    { "dupn",       OPERAND_UINT32 },
    { "enum",       OPERAND_NONE },
    { "enumnext",   OPERAND_NONE },
    { "jend",       OPERAND_UINT32 },
    { "enumpop",    OPERAND_NONE },
    { "eq",         OPERAND_NONE },
};

js_instruction_t* js_instruction(uint32_t opcode)
{
    if(opcode >= sizeof(insns) / sizeof(insns[0])) {
        return NULL;
    }
    return &insns[opcode];
}

static void* stack_limit;

void js_vm_set_stack_limit(void* stack_limit_)
{
    stack_limit = stack_limit_;
}

js_vm_t* js_vm_new()
{
    js_vm_t* vm = js_alloc(sizeof(js_vm_t));
    // this proto/constructor is fixed up later by js_lib_initialize
    vm->global_scope = js_scope_make_global(vm, js_value_make_object(js_value_undefined(), js_value_undefined()));
    js_object_put(vm->global_scope->global_object, js_cstring("global"), vm->global_scope->global_object);
    js_lib_initialize(vm);
    return vm;
}

static int comparison_oper(VAL left, VAL right)
{
    double l, r;
    if(js_value_get_type(left) == JS_T_STRING && js_value_get_type(right) == JS_T_STRING) {
        return js_string_cmp(&js_value_get_pointer(left)->string, &js_value_get_pointer(right)->string);
    } else {
        l = js_value_get_double(js_to_number(left));
        r = js_value_get_double(js_to_number(right));
        if(l < r) {
            return -1;
        } else if(l > r) {
            return 1;
        } else {
            return 0;
        }
    }
}

/* @TODO: bounds checking here */
#define NEXT_UINT32() (L->INSNS[L->IP++])
#define NEXT_DOUBLE() (L->IP += 2, *(double*)&L->INSNS[L->IP - 2])
#define NEXT_STRING() (L->image->strings[NEXT_UINT32()])

static int popped_under_zero_hack() {
    js_panic("popped SP < 0");
}

#define PUSH(v) do { \
                    if(L->SP >= L->SMAX) { \
                        L->SMAX *= 2; \
                        if(L->STACK_CSTACK) L->STACK = memcpy(js_alloc(L->SMAX / 2 * sizeof(VAL)), L->STACK, L->SMAX / 2 * sizeof(VAL)); \
                        L->STACK = js_realloc(L->STACK, sizeof(VAL) * L->SMAX); \
                    } \
                    L->STACK[L->SP++] = (v); \
                } while(false)
#define POP()   (L->STACK[--L->SP < 0 ? popped_under_zero_hack() : L->SP])
#define PEEK()  (L->STACK[L->SP - 1])

static uint32_t global_instruction_counter = 0;

struct exception_frame {
    uint32_t catch;
    uint32_t finally;
    struct exception_frame* prev;
};

struct enum_frame {
    js_string_t** keys;
    uint32_t count;
    uint32_t index;
    struct enum_frame* prev;
};

struct vm_locals {
    js_vm_t* vm;
    js_image_t* image;
    uint32_t section;
    js_scope_t* scope;
    VAL this;
    uint32_t argc;
    VAL* argv;
    
    uint32_t IP;
    uint32_t* INSNS;
    int32_t SP;
    int32_t SMAX;
    bool STACK_CSTACK;
    VAL* STACK;
    VAL temp_slot;
    uint32_t current_line;
    
    bool exception_thrown;
    bool return_after_finally;
    bool will_return;
    VAL return_val;
    VAL return_after_finally_val;
    VAL exception;
    struct exception_frame* exception_stack;
    js_exception_handler_t handler;
    
    struct enum_frame* enum_stack;
};

static VAL vm_exec(struct vm_locals* L);

int kprintf();

VAL js_vm_exec(js_vm_t* vm, js_image_t* image, uint32_t section, js_scope_t* scope, VAL this, uint32_t argc, VAL* argv)
{
    VAL fast_stack[32];
    
    struct vm_locals L;
    
    L.vm = vm;
    L.image = image;
    L.section = section;
    L.scope = scope;
    L.this = this;
    L.argc = argc;
    L.argv = argv;
    
    L.IP = 0;
    L.INSNS = image->sections[section].instructions;
    
    L.SP = 0;
    L.SMAX = sizeof(fast_stack) / sizeof(VAL);
    L.STACK_CSTACK = true;
    L.STACK = fast_stack;
    L.temp_slot = js_value_undefined();
    L.current_line = 1;
    
    L.exception_stack = NULL;
    L.exception_thrown = false;
    L.return_after_finally = false;
    L.will_return = false;
    L.return_val = js_value_undefined();
    L.return_after_finally_val = js_value_undefined();
    L.exception = js_value_undefined();
    
    L.enum_stack = NULL;
    
    if((uint32_t)&L < (uint32_t)stack_limit) {
        js_throw_error(vm->lib.RangeError, "Stack overflow");
    }
    
    L.handler.previous = js_current_exception_handler();
    js_set_exception_handler(&L.handler);
    VAL retn = vm_exec(&L);
    js_set_exception_handler(L.handler.previous);
    return retn;
}

static VAL vm_exec(struct vm_locals* L)
{
    uint32_t opcode;
    
    if(setjmp(L->handler.env)) {
        // exception was thrown
        L->exception_thrown = true;
        L->exception = L->handler.exception;
        if(js_value_is_object(L->handler.exception)) {
            js_string_t* trace = js_value_get_pointer(L->handler.exception)->object.stack_trace;
            trace = js_string_concat(trace, js_string_format("\n    at %s:%d", L->image->strings[L->image->name]->buff, L->current_line));
            js_value_get_pointer(L->handler.exception)->object.stack_trace = trace;
        }
        if(L->exception_stack) {
            if(L->exception_stack->catch) {
                L->IP = L->exception_stack->catch;
            } else {
                L->IP = L->exception_stack->finally;
            }
        } else {
            js_set_exception_handler(L->handler.previous);
            js_throw(L->handler.exception);
        }
    }
    
    while(1) {
        if(++global_instruction_counter >= VM_CYCLES_PER_COLLECTION) {
            global_instruction_counter = 0;
            js_gc_run();
        }
        if(L->will_return) {
            return L->return_val;
        }
        opcode = NEXT_UINT32();
        switch(opcode) {
            case JS_OP_UNDEFINED: {
                PUSH(js_value_undefined());
                break;
            }
        
            case JS_OP_RET: {
                if(L->exception_stack) {
                    L->return_after_finally_val = POP();
                    L->return_after_finally = true;
                    L->IP = L->exception_stack->finally;
                } else {
                    return POP();
                }
                break;
            }
        
            case JS_OP_PUSHNUM: {
                double d = NEXT_DOUBLE();
                PUSH(js_value_make_double(d));
                break;
            }
        
            case JS_OP_ADD: {
                VAL r = js_to_primitive(POP());
                VAL l = js_to_primitive(POP());
                if(js_value_get_type(l) == JS_T_STRING || js_value_get_type(r) == JS_T_STRING) {
                    js_string_t* sl = &js_value_get_pointer(js_to_string(l))->string;
                    js_string_t* sr = &js_value_get_pointer(js_to_string(r))->string;
                    PUSH(js_value_wrap_string(js_string_concat(sl, sr)));
                } else {
                    PUSH(js_value_make_double(js_value_get_double(js_to_number(l)) + js_value_get_double(js_to_number(r))));
                }
                break;
            }
        
            case JS_OP_PUSHGLOBAL: {
                js_string_t* var = NEXT_STRING();
                PUSH(js_scope_get_global_var(L->scope, var));
                break;
            }
    
            case JS_OP_PUSHSTR: {
                js_string_t* str = NEXT_STRING();
                PUSH(js_value_wrap_string(str));
                break;
            }
    
            case JS_OP_METHCALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL argv[argc];
                VAL method, obj, fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                method = POP();
                obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(L->vm, obj);
                }
                fn = js_object_get(obj, js_to_js_string_t(method));
                if(js_value_get_type(fn) != JS_T_FUNCTION) {
                    js_throw_error(L->vm->lib.TypeError, "called non callable");
                }
                PUSH(js_call(fn, obj, argc, argv));
                break;
            }
    
            case JS_OP_SETVAR: {
                uint32_t idx = NEXT_UINT32();
                uint32_t sc = NEXT_UINT32();
                js_scope_set_var(L->scope, idx, sc, PEEK());
                break;
            }
    
            case JS_OP_PUSHVAR: {
                uint32_t idx = NEXT_UINT32();
                uint32_t sc = NEXT_UINT32();
                PUSH(js_scope_get_var(L->scope, idx, sc));
                break;
            }
        
            case JS_OP_TRUE: {
                PUSH(js_value_true());
                break;
            }
            
            case JS_OP_FALSE: {
                PUSH(js_value_false());
                break;
            }

            case JS_OP_NULL: {
                PUSH(js_value_null());
                break;
            }
            
            case JS_OP_JMP: {
                uint32_t next = NEXT_UINT32();
                L->IP = next;
                break;
            }
            
            case JS_OP_JIT: {
                uint32_t next = NEXT_UINT32();
                if(js_value_is_truthy(POP())) {
                    L->IP = next;
                }
                break;
            }
        
            case JS_OP_JIF: {
                uint32_t next = NEXT_UINT32();
                if(!js_value_is_truthy(POP())) {
                    L->IP = next;
                }
                break;
            }
        
            case JS_OP_SUB: {
                VAL r = js_to_number(POP());
                VAL l = js_to_number(POP());
                PUSH(js_value_make_double(js_value_get_double(js_to_number(l)) - js_value_get_double(js_to_number(r))));
                break;
            }
        
            case JS_OP_MUL: {
                VAL r = js_to_number(POP());
                VAL l = js_to_number(POP());
                PUSH(js_value_make_double(js_value_get_double(js_to_number(l)) * js_value_get_double(js_to_number(r))));
                break;
            }
        
            case JS_OP_DIV: {
                VAL r = js_to_number(POP());
                VAL l = js_to_number(POP());
                PUSH(js_value_make_double(js_value_get_double(js_to_number(l)) / js_value_get_double(js_to_number(r))));
                break;
            }
        
            case JS_OP_SETGLOBAL: {
                js_string_t* str = NEXT_STRING();
                js_scope_set_global_var(L->scope, str, PEEK());
                break;
            }
        
            case JS_OP_CLOSE: {
                uint32_t sect = NEXT_UINT32();
                PUSH(js_value_make_function(L->vm, L->image, sect, L->scope));
                break;
            }

            case JS_OP_CALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL argv[argc];
                VAL fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                fn = POP();
                if(js_value_get_type(fn) != JS_T_FUNCTION) {
                    js_throw_error(L->vm->lib.TypeError, "called non callable");
                }
                PUSH(js_call(fn, L->vm->global_scope->global_object, argc, argv));
                break;
            }
        
            case JS_OP_SETCALLEE: {
                uint32_t idx = NEXT_UINT32();
                if(L->scope->parent) { /* not global scope... */
                    js_scope_set_var(L->scope, idx, 0, L->scope->locals.callee);
                }
                break;
            }
        
            case JS_OP_SETARG: {
                uint32_t var = NEXT_UINT32();
                uint32_t arg = NEXT_UINT32();
                if(L->scope->parent) { /* not global scope... */
                    if(arg >= L->argc) {
                        js_scope_set_var(L->scope, var, 0, js_value_undefined());
                    } else {
                        js_scope_set_var(L->scope, var, 0, L->argv[arg]);
                    }
                }
                break;
            }
        
            case JS_OP_LT: {
                VAL right = POP();
                VAL left = POP();
                PUSH(js_value_make_boolean(comparison_oper(left, right) < 0));
                break;
            }
        
            case JS_OP_LTE: {
                VAL right = POP();
                VAL left = POP();
                PUSH(js_value_make_boolean(comparison_oper(left, right) <= 0));
                break;
            }
        
            case JS_OP_GT: {
                VAL right = POP();
                VAL left = POP();
                PUSH(js_value_make_boolean(comparison_oper(left, right) > 0));
                break;
            }
        
            case JS_OP_GTE: {
                VAL right = POP();
                VAL left = POP();
                PUSH(js_value_make_boolean(comparison_oper(left, right) >= 0));
                break;
            }
        
            case JS_OP_POP: {
                (void)POP();
                break;
            }
        
            case JS_OP_ARRAY: {
                uint32_t i, count = NEXT_UINT32();
                VAL items[count];
                for(i = 0; i < count; i++) {
                    items[count - i - 1] = POP();
                }
                PUSH(js_make_array(L->vm, count, items));
                break;
            }
        
            case JS_OP_NEWCALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL argv[argc];
                VAL fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                fn = POP();
                if(js_value_get_type(fn) != JS_T_FUNCTION) {
                    js_throw_error(L->vm->lib.TypeError, "constructed non callable");
                }
                PUSH(js_construct(fn, argc, argv));
                break;
            }
        
            case JS_OP_THROW: {
                js_throw(POP());
                break;
            };
        
            case JS_OP_MEMBER: {
                js_string_t* member = NEXT_STRING();
                VAL obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(L->vm, obj);
                }
                PUSH(js_object_get(obj, member));
                break;
            }
        
            case JS_OP_DUP: {
                VAL v = PEEK();
                PUSH(v);
                break;
            }
        
            case JS_OP_THIS: {
                PUSH(L->this);
                break;
            }
        
            case JS_OP_SETPROP: {
                VAL val = POP();
                VAL obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(L->vm, obj);
                }
                js_object_put(obj, NEXT_STRING(), val);
                PUSH(val);
                break;
            }
        
            case JS_OP_TST: {
                L->temp_slot = POP();
                break;
            }
        
            case JS_OP_TLD: {
                PUSH(L->temp_slot);
                break;
            }
        
            case JS_OP_INDEX: {
                VAL index = js_to_string(POP());
                VAL object = POP();
                if(js_value_is_primitive(object)) {
                    object = js_to_object(L->vm, object);
                }
                PUSH(js_object_get(object, &js_value_get_pointer(index)->string));
                break;
            }
        
            case JS_OP_SETINDEX: {
                VAL val = POP();
                VAL idx = js_to_string(POP());
                VAL obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(L->vm, obj);
                }
                js_object_put(obj, js_to_js_string_t(idx), val);
                PUSH(val);
                break;
            }
        
            case JS_OP_OBJECT: {
                uint32_t i, items = NEXT_UINT32();
                VAL obj = js_make_object(L->vm);
                for(i = 0; i < items; i++) {
                    VAL val = POP();
                    VAL key = js_to_string(POP());
                    js_object_put(obj, js_to_js_string_t(key), val);
                }
                PUSH(obj);
                break;
            }
        
            case JS_OP_TYPEOF: {
                VAL val = POP();
                PUSH(js_typeof(val));
                break;
            }
        
            case JS_OP_SEQ: {
                VAL r = POP();
                VAL l = POP();
                PUSH(js_value_make_boolean(js_seq(l, r)));
                break;
            }
        
            case JS_OP_TYPEOFG: {
                js_string_t* var = NEXT_STRING();
                if(js_scope_global_var_exists(L->scope, var)) {
                    PUSH(js_typeof(js_scope_get_global_var(L->scope, var)));
                } else {
                    PUSH(js_value_make_cstring("undefined"));
                }
                break;
            }
        
            case JS_OP_SAL: {
                uint32_t r = js_to_uint32(POP());
                uint32_t l = js_to_uint32(POP());
                PUSH(js_value_make_double(l << r));
                break;
            }
        
            case JS_OP_OR: {
                uint32_t r = js_to_uint32(POP());
                uint32_t l = js_to_uint32(POP());
                PUSH(js_value_make_double(l | r));
                break;
            }
        
            case JS_OP_XOR: {
                uint32_t r = js_to_uint32(POP());
                uint32_t l = js_to_uint32(POP());
                PUSH(js_value_make_double(l ^ r));
                break;
            }
        
            case JS_OP_AND: {
                uint32_t r = js_to_uint32(POP());
                uint32_t l = js_to_uint32(POP());
                PUSH(js_value_make_double(l & r));
                break;
            }
        
            case JS_OP_SLR: {
                uint32_t r = js_to_uint32(POP());
                uint32_t l = js_to_uint32(POP());
                PUSH(js_value_make_double(l >> r));
                break;
            }
        
            case JS_OP_NOT: {
                VAL v = POP();
                PUSH(js_value_make_boolean(!js_value_is_truthy(v)));
                break;
            }
        
            case JS_OP_BITNOT: {
                uint32_t x = js_to_uint32(POP());
                PUSH(js_value_make_double(~x));
                break;
            }
            
            case JS_OP_LINE: {
                L->current_line = NEXT_UINT32();
                break;
            }
        
            case JS_OP_DEBUGGER: {
                js_panic("DEBUGGER");
                break;
            }
            
            case JS_OP_INSTANCEOF: {
                VAL class = POP();
                VAL obj = POP();
                if(js_value_get_type(class) != JS_T_FUNCTION) {
                    js_throw_error(L->vm->lib.TypeError, "expected a function in instanceof check");
                }
                if(js_value_is_primitive(obj)) {
                    PUSH(js_value_false());
                } else {
                    PUSH(js_value_make_boolean(js_value_get_pointer(js_value_get_pointer(obj)->object.class) == js_value_get_pointer(class)));
                }
                break;
            }
            
            case JS_OP_NEGATE: {
                double d = js_value_get_double(js_to_number(POP()));
                PUSH(js_value_make_double(-d));
                break;
            }
            
            case JS_OP_TRY: {
                uint32_t catch = NEXT_UINT32();
                uint32_t finally = NEXT_UINT32();
                struct exception_frame* frame = js_alloc(sizeof(struct exception_frame));
                frame->catch = catch;
                frame->finally = finally;
                frame->prev = L->exception_stack;
                L->exception_stack = frame;
                break;
            }
            
            case JS_OP_POPTRY: {
                struct exception_frame* frame = L->exception_stack;
                L->IP = frame->finally;
                L->exception_stack = frame->prev;
                break;
            }
            
            case JS_OP_CATCH: {
                L->exception_stack->catch = 0;
                js_scope_set_var(L->scope, NEXT_UINT32(), 0, L->exception);
                L->exception = js_value_undefined();
                L->exception_thrown = false;
                break;
            }
            
            case JS_OP_CATCHG: {
                L->exception_stack->catch = 0;
                js_scope_set_global_var(L->scope, NEXT_STRING(), L->exception);
                L->exception = js_value_undefined();
                L->exception_thrown = false;
                break;
            }
            
            case JS_OP_POPCATCH: {
                L->IP = L->exception_stack->finally;
                break;
            }
            
            case JS_OP_FINALLY: {
                break;
            }
            
            case JS_OP_POPFINALLY: {
                if(L->exception_thrown) {
                    js_throw(L->exception);
                }
                if(L->return_after_finally) {
                    L->return_after_finally = false;
                    L->will_return = true;
                    L->return_val = L->return_after_finally_val;
                    L->return_after_finally_val = js_value_undefined();
                }
                break;
            }
            
            case JS_OP_CLOSENAMED: {
                uint32_t sect = NEXT_UINT32();
                js_string_t* name = NEXT_STRING();
                VAL function = js_value_make_function(L->vm, L->image, sect, L->scope);
                ((js_function_t*)js_value_get_pointer(function))->name = name;
                PUSH(function);
                break;
            }
            
            case JS_OP_DELETE: {
                VAL index = js_to_string(POP());
                VAL object = POP();
                if(js_value_is_primitive(object)) {
                    object = js_to_object(L->vm, object);
                }
                PUSH(js_value_make_boolean(js_object_delete(object, js_to_js_string_t(index))));
                break;
            }
            
            case JS_OP_MOD: {
                VAL r = js_to_number(POP());
                VAL l = js_to_number(POP());
                PUSH(js_value_make_double(fmod(js_value_get_double(js_to_number(l)), js_value_get_double(js_to_number(r)))));
                break;
            }
            
            case JS_OP_ARGUMENTS: {
                uint32_t idx = NEXT_UINT32();
                VAL arguments = js_make_array(L->vm, L->argc, L->argv);
                js_object_put(arguments, js_cstring("callee"), L->scope->locals.callee);
                js_scope_set_var(L->scope, idx, 0, arguments);
                break;
            }
            
            case JS_OP_DUPN: {
                uint32_t n = NEXT_UINT32();
                VAL dup[n];
                uint32_t i;
                for(i = 0; i < n; i++) {
                    dup[i] = POP();
                }
                for(i = n; i > 0; i--) {
                    PUSH(dup[i - 1]);
                }
                for(i = n; i > 0; i--) {
                    PUSH(dup[i - 1]);
                }
                break;
            }
            
            case JS_OP_ENUM: {
                struct enum_frame* frame = js_alloc(sizeof(struct enum_frame));
                frame->prev = L->enum_stack;
                frame->index = 0;
                frame->keys = js_object_keys(js_to_object(L->vm, POP()), &frame->count);
                L->enum_stack = frame;
                break;
            }
            
            case JS_OP_ENUMNEXT: {
                PUSH(js_value_wrap_string(L->enum_stack->keys[L->enum_stack->index++]));
                break;
            }
            
            case JS_OP_JEND: {
                uint32_t ip = NEXT_UINT32();
                if(L->enum_stack->index == L->enum_stack->count) {
                    L->IP = ip;
                }
                break;
            }
            
            case JS_OP_ENUMPOP: {
                L->enum_stack = L->enum_stack->prev;
                break;
            }
            
            case JS_OP_EQ: {
                VAL r = POP();
                VAL l = POP();
                PUSH(js_value_make_boolean(js_eq(L->vm, l, r)));
                break;
            }
        
            default:
                /* @TODO proper-ify this */
                js_panic("unknown opcode %u\n", opcode);
        }
    }
}
