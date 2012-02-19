#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lib.h"
#include "object.h"
#include "gc.h"
#include "exception.h"

typedef struct {
    js_value_t base;
    double number;
} js_number_object_t;

static VAL Number_call(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_value_make_double(0);
    } else {
        return js_to_number(argv[0]);
    }
}

static VAL Number_construct(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0) {
        return js_make_number_object(vm, 0);
    } else {
        return js_make_number_object(vm, js_value_get_double(js_to_number(argv[0])));
    }
}

VAL js_make_number_object(js_vm_t* vm, double number)
{
    js_number_object_t* num = js_alloc(sizeof(js_number_object_t));
    num->base.type = JS_T_NUMBER_OBJECT;
    num->base.object.vtable = js_object_base_vtable();
    num->base.object.prototype = vm->lib.Number_prototype;
    num->base.object.class = vm->lib.Number;
    num->base.object.properties = js_st_table_new();
    num->number = number;
    return js_value_make_pointer((js_value_t*)num);
}

static VAL Number_prototype_toString(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_NUMBER_OBJECT) {
        // @TODO throw exception
        js_panic("Number.prototype.toString() is not generic");
    }
    return js_to_string(js_value_make_double(((js_number_object_t*)js_value_get_pointer(this))->number));
}

static VAL Number_prototype_valueOf(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(js_value_get_type(this) != JS_T_NUMBER_OBJECT) {
        js_panic("Number.prototype.valueOf() is not generic");
    }
    return js_value_make_double(((js_number_object_t*)js_value_get_pointer(this))->number);
}

void js_lib_number_initialize(js_vm_t* vm)
{
    vm->lib.Number = js_value_make_native_function(vm, NULL, js_cstring("Number"), Number_call, Number_construct);
    js_object_put(vm->global_scope->global_object, js_cstring("Number"), vm->lib.Number);
    
    vm->lib.Number_prototype = js_value_make_object(vm->lib.Object_prototype, vm->lib.Number);
    js_object_put(vm->lib.Number, js_cstring("prototype"), vm->lib.Number_prototype);
    
    js_object_put(vm->lib.Number_prototype, js_cstring("toString"), js_value_make_native_function(vm, NULL, js_cstring("toString"), Number_prototype_toString, NULL));
    js_object_put(vm->lib.Number_prototype, js_cstring("valueOf"), js_value_make_native_function(vm, NULL, js_cstring("valueOf"), Number_prototype_valueOf, NULL));
}

double js_number_parse(js_string_t* str)
{
    // parse str according to ECMA-262 Section 9.3.1
    double number = 0;
    double frac = 0.1;
    int exponent = 0;
    bool neg = false, negexp = false;
    uint32_t i;
    for(i = 0; i < str->length; i++) {
        if(!isspace(str->buff[i])) {
            break;
        }
    }
    if(i == str->length) {
        return NAN;
    }
    if(str->buff[i] == '0' && i + 1 < str->length && (str->buff[i + 1] == 'x' || str->buff[i + 1] == 'X')) {
        // hex number:
        i += 2;
        if(i == str->length) return NAN;
        for(; i < str->length; i++) {
            number *= 16;
            if(str->buff[i] >= '0' && str->buff[i] <= '9') {
                number += str->buff[i] - '0';
            } else if(str->buff[i] >= 'a' && str->buff[i] <= 'f') {
                number += 10 + str->buff[i] - 'a';
            } else if(str->buff[i] >= 'A' && str->buff[i] <= 'F') {
                number += 10 + str->buff[i] - 'A';
            } else if(isspace(str->buff[i])) {
                goto whitespace_only;
            } else {
                return NAN;
            }
        }
    } else {
        if(i + 8 <= str->length && memcmp(str->buff + i, "Infinity", 8) == 0) {
            number = INFINITY;
            goto whitespace_only;
        }
        if(str->buff[i] == '-') {
            neg = true;
            i++;
        }
        if(str->buff[i] == '+') {
            i++;
        }
        if(i == str->length) {
            return NAN;
        }
        for(; i < str->length; i++) {
            if(str->buff[i] >= '0' && str->buff[i] <= '9') {
                number *= 10;
                number += str->buff[i] - '0';
            } else {
                break;
            }
        }
        if(i == str->length) {
            return (neg ? -1.0 : 1.0) * number;
        }
        if(str->buff[i] == '.') {
            i++;
            for(; i < str->length; i++) {
                if(str->buff[i] >= '0' && str->buff[i] <= '9') {
                    number += (str->buff[i] - '0') * frac;
                    frac /= 10.0;
                } else {
                    break;
                }
            }
        }
        if(i == str->length) {
            return (neg ? -1.0 : 1.0) * number;
        }
        if(str->buff[i] == 'e' || str->buff[i] == 'E') {
            i++;
            if(i < str->length) {
                if(str->buff[i] == '+') {
                    i++;
                } else if(str->buff[i] == '-') {
                    negexp = true;
                    i++;
                }
            }
            for(; i < str->length; i++) {
                if(str->buff[i] >= '0' && str->buff[i] <= '9') {
                    exponent *= 10;
                    exponent += str->buff[i] - '0';
                } else {
                    break;
                }
            }
            if(negexp) {
                exponent = -exponent;
            }
            while(exponent > 0) {
                number *= 10.0;
                exponent--;
            }
            while(exponent < 0) {
                number /= 10.0;
                exponent++;
            }
            goto whitespace_only;
        }
    }
    
whitespace_only:
    while(i < str->length) {
        if(!isspace(str->buff[i])) {
            return NAN;
        }
    }
    return (neg ? -1.0 : 1.0) * number;
}