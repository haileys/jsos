#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
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
};

js_instruction_t* js_instruction(uint32_t opcode)
{
    if(opcode >= sizeof(insns) / sizeof(insns[0])) {
        return NULL;
    }
    return &insns[opcode];
}

js_vm_t* js_vm_new()
{
    js_vm_t* vm = js_alloc(sizeof(js_vm_t));
    // this proto/constructor is fixed up later by js_lib_initialize
    vm->global_scope = js_scope_make_global(js_value_make_object(js_value_undefined(), js_value_undefined()));
    js_object_put(vm->global_scope->global_object, js_cstring("global"), vm->global_scope->global_object);
    js_lib_initialize(vm);
    return vm;
}

/* @TODO: bounds checking here */
#define NEXT_UINT32() (INSNS[IP++])
#define NEXT_DOUBLE() (IP += 2, *(double*)&INSNS[IP - 2])
#define NEXT_STRING() (&image->strings[NEXT_UINT32()])

#define PUSH(v) do { \
                    if(SP >= SMAX) { \
                        STACK = js_realloc(STACK, sizeof(VAL) * (SMAX *= 2)); \
                    } \
                    STACK[SP++] = (v); \
                } while(false)
#define POP()   (STACK[--SP])
#define PEEK()  (STACK[SP - 1])

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

VAL js_vm_exec(js_vm_t* vm, js_image_t* image, uint32_t section, js_scope_t* scope, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t IP = 0;
    uint32_t* INSNS = image->sections[section].instructions;
    uint32_t opcode;
    
    uint32_t SP = 0;
    uint32_t SMAX = 8;
    VAL* STACK = js_alloc(sizeof(VAL) * SMAX);
    VAL temp_slot = js_value_undefined();
    
    while(1) {
        //js_gc_run();
        opcode = NEXT_UINT32();
        switch(opcode) {
            case JS_OP_UNDEFINED:
                PUSH(js_value_undefined());
                break;
            
            case JS_OP_RET:
                return POP();
                break;
            
            case JS_OP_PUSHNUM: {
                double d = NEXT_DOUBLE();
                PUSH(js_value_make_double(d));
                break;
            }
            
            case JS_OP_ADD: {
                VAL r = js_to_primitive(POP());
                VAL l = js_to_primitive(POP());
                if(js_value_get_type(l) == JS_T_STRING || js_value_get_type(r) == JS_T_STRING) {
                    js_string_t* sl = &js_value_get_pointer(l)->string;
                    js_string_t* sr = &js_value_get_pointer(r)->string;
                    PUSH(js_value_wrap_string(js_string_concat(sl, sr)));
                } else {
                    PUSH(js_value_make_double(js_value_get_double(js_to_number(l)) + js_value_get_double(js_to_number(r))));
                }
                break;
            }
            
            case JS_OP_PUSHGLOBAL: {
                js_string_t* var = NEXT_STRING();
                PUSH(js_scope_get_global_var(scope, var));
                break;
            }
        
            case JS_OP_PUSHSTR: {
                js_string_t* str = NEXT_STRING();
                PUSH(js_value_wrap_string(str));
                break;
            }
        
            case JS_OP_METHCALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL* argv = js_alloc(sizeof(VAL) * argc);
                VAL method, obj, fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                method = POP();
                obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(vm, obj);
                }
                fn = js_object_get(obj, &js_value_get_pointer(js_to_string(method))->string);
                PUSH(js_call(fn, obj, argc, argv));
                break;
            }
        
            case JS_OP_SETVAR: {
                uint32_t idx = NEXT_UINT32();
                uint32_t sc = NEXT_UINT32();
                js_scope_set_var(scope, idx, sc, PEEK());
                break;
            }
        
            case JS_OP_PUSHVAR: {
                uint32_t idx = NEXT_UINT32();
                uint32_t sc = NEXT_UINT32();
                PUSH(js_scope_get_var(scope, idx, sc));
                break;
            }
            
            case JS_OP_TRUE:
                PUSH(js_value_true());
                break;
                
            case JS_OP_FALSE:
                PUSH(js_value_false());
                break;

            case JS_OP_NULL:
                PUSH(js_value_null());
                break;
                
            case JS_OP_JMP: {
                uint32_t next = NEXT_UINT32();
                IP = next;
                break;
            }
                
            case JS_OP_JIT: {
                uint32_t next = NEXT_UINT32();
                if(js_value_is_truthy(POP())) {
                    IP = next;
                }
                break;
            }
            
            case JS_OP_JIF: {
                uint32_t next = NEXT_UINT32();
                if(!js_value_is_truthy(POP())) {
                    IP = next;
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
                js_scope_set_global_var(scope, str, PEEK());
                break;
            }
            
            case JS_OP_CLOSE: {
                uint32_t sect = NEXT_UINT32();
                PUSH(js_value_make_function(vm, image, sect, scope));
                break;
            }

            case JS_OP_CALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL* argv = js_alloc(sizeof(VAL) * argc);
                VAL fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                fn = POP();
                PUSH(js_call(fn, vm->global_scope->global_object, argc, argv));
                break;
            }
            
            case JS_OP_SETCALLEE: {
                uint32_t idx = NEXT_UINT32();
                if(scope->parent) { /* not global scope... */
                    js_scope_set_var(scope, idx, 0, scope->locals.callee);
                }
                break;
            }
            
            case JS_OP_SETARG: {
                uint32_t var = NEXT_UINT32();
                uint32_t arg = NEXT_UINT32();
                if(scope->parent) { /* not global scope... */
                    if(arg >= argc) {
                        js_scope_set_var(scope, var, 0, js_value_undefined());
                    } else {
                        js_scope_set_var(scope, var, 0, argv[arg]);
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
            
            case JS_OP_POP:
                (void)POP();
                break;
            
            case JS_OP_ARRAY: {
                uint32_t i, count = NEXT_UINT32();
                VAL* items = js_alloc(sizeof(VAL) * count);
                for(i = 0; i < count; i++) {
                    items[count - i - 1] = POP();
                }
                PUSH(js_make_array(vm, count, items));
                break;
            }
            
            case JS_OP_NEWCALL: {
                uint32_t i, argc = NEXT_UINT32();
                VAL* argv = js_alloc(sizeof(VAL) * argc);
                VAL fn;
                for(i = 0; i < argc; i++) {
                    argv[argc - i - 1] = POP();
                }
                fn = POP();
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
                    obj = js_to_object(vm, obj);
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
                PUSH(this);
                break;
            }
            
            case JS_OP_SETPROP: {
                VAL val = POP();
                VAL obj = POP();
                if(js_value_is_primitive(obj)) {
                    obj = js_to_object(vm, obj);
                }
                js_object_put(obj, NEXT_STRING(), val);
                PUSH(val);
                break;
            }
            
            case JS_OP_TST: {
                temp_slot = POP();
                break;
            }
            
            case JS_OP_TLD: {
                PUSH(temp_slot);
                break;
            }
            
            case JS_OP_INDEX: {
                VAL index = js_to_string(POP());
                VAL object = POP();
                if(js_value_is_primitive(object)) {
                    object = js_to_object(vm, object);
                }
                PUSH(js_object_get(object, &js_value_get_pointer(index)->string));
                break;
            }
            
            default:
                /* @TODO proper-ify this */
                js_panic("unknown opcode %u\n", opcode);
        }
    }
}