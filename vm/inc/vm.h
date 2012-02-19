#ifndef JS_VM_H
#define JS_VM_H

#include <stdint.h>
#include "scope.h"
#include "image.h"
#include "value.h"
#include "lib.h"

typedef struct js_vm {
    js_scope_t* global_scope;
    js_lib_t lib;
} js_vm_t;

js_vm_t* js_vm_new();
VAL js_vm_exec(js_vm_t* vm, js_image_t* image, uint32_t section, js_scope_t* scope, VAL this, uint32_t argc, VAL* argv);

enum js_opcode {
    JS_OP_UNDEFINED     = 0,
    JS_OP_RET           = 1,
    JS_OP_PUSHNUM       = 2,
    JS_OP_ADD           = 3,
    JS_OP_PUSHGLOBAL    = 4,
    JS_OP_PUSHSTR       = 5,
    JS_OP_METHCALL      = 6,
    JS_OP_SETVAR        = 7,
    JS_OP_PUSHVAR       = 8,
    JS_OP_TRUE          = 9,
    JS_OP_FALSE         = 10,
    JS_OP_NULL          = 11,
    JS_OP_JMP           = 12,
    JS_OP_JIT           = 13,
    JS_OP_JIF           = 14,
    JS_OP_SUB           = 15,
    JS_OP_MUL           = 16,
    JS_OP_DIV           = 17,
    JS_OP_SETGLOBAL     = 18,
    JS_OP_CLOSE         = 19,
    JS_OP_CALL          = 20,
    JS_OP_SETCALLEE     = 21,
    JS_OP_SETARG        = 22,
    JS_OP_LT            = 23,
    JS_OP_LTE           = 24,
    JS_OP_GT            = 25,
    JS_OP_GTE           = 26,
    JS_OP_POP           = 27,
    JS_OP_ARRAY         = 28,
    JS_OP_NEWCALL       = 29,
    JS_OP_THROW         = 30,
    JS_OP_MEMBER        = 31,
    JS_OP_DUP           = 32,
    JS_OP_THIS          = 33,
    JS_OP_SETPROP       = 34,
    JS_OP_TST           = 35,
    JS_OP_TLD           = 36,
    JS_OP_INDEX         = 37,
};

typedef struct {
    char* name;
    enum {
        OPERAND_NONE,
        OPERAND_NUMBER,
        OPERAND_UINT32,
        OPERAND_UINT32_UINT32,
        OPERAND_STRING,
    } operand;
} js_instruction_t;

js_instruction_t* js_instruction(uint32_t opcode);

#endif