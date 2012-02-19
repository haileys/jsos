#ifndef JS_LIB_H
#define JS_LIB_H

#include "value.h"

typedef struct {
    VAL Function;
    VAL Function_prototype;
    VAL Object;
    VAL Object_prototype;
    VAL Array;
    VAL Array_prototype;
    VAL Number;
    VAL Number_prototype;
    VAL Error;
    VAL Error_prototype;
    VAL RangeError;
    VAL RangeError_prototype;
    VAL ReferenceError;
    VAL ReferenceError_prototype;
    VAL TypeError;
    VAL TypeError_prototype;
} js_lib_t;

#include "vm.h"

void js_lib_initialize(struct js_vm* vm);

/* Function */
void js_lib_function_initialize(struct js_vm* vm);

/* Object */
void js_lib_object_initialize(struct js_vm* vm);
VAL js_make_object(struct js_vm* vm);

/* Array */
void js_lib_array_initialize(struct js_vm* vm);
VAL js_make_array(struct js_vm* vm, uint32_t count, VAL* items);

/* Number */
void js_lib_number_initialize(struct js_vm* vm);
VAL js_make_number_object(struct js_vm* vm, double number);
double js_number_parse(js_string_t* str);

/* Error */
void js_lib_error_initialize(struct js_vm* vm);
VAL js_make_error(VAL class, js_string_t* message);
void js_throw_error(VAL class, char* fmt, ...);

#endif