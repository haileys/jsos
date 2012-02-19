#ifndef JS_VALUE_H
#define JS_VALUE_H

#include <stdbool.h>
#include <stdint.h>
#include "st.h"

typedef enum {
    JS_T_NULL,
    JS_T_UNDEFINED,
    JS_T_BOOLEAN,
    JS_T_NUMBER,
    JS_T_OBJECT,
    JS_T_STRING,
    JS_T_FUNCTION,
    JS_T_ARRAY,
    JS_T_STRING_OBJECT,
    JS_T_NUMBER_OBJECT,
    JS_T_BOOLEAN_OBJECT,
} js_type_t;

typedef union {
    double d;
    uint64_t i;
} VAL;

typedef struct {
    uint32_t length;
    char* buff;
} js_string_t;

typedef struct {
    VAL value;
    /*
    bool is_accessor;
    bool enumerable;
    bool configurable;
    union {
        struct {
            VAL value;
            bool writable;
        } data;
        struct {
            VAL get;
            VAL set;
        } accessor;
    };
    */
} js_property_descriptor_t;

struct js_object_internal_methods;

typedef struct {
    struct js_object_internal_methods* vtable;
    VAL prototype;
    VAL class;
    st_table* properties;
} js_object_t;

typedef struct {
    js_type_t type;
    union {
        js_string_t string;
        js_object_t object;
    };
} js_value_t;

typedef struct {
    js_value_t base;
    bool is_native;
    struct js_vm* vm;
    js_string_t* name;
    union {
        struct {
            void* state;
            VAL(*call)(struct js_vm*, void*, VAL, uint32_t, VAL*);
            VAL(*construct)(struct js_vm*, void*, VAL, uint32_t, VAL*);
        } native;
        struct {
            struct js_image* image;
            uint32_t section;
            struct js_scope* outer_scope;
        } js;
    };
} js_function_t;

typedef struct js_object_internal_methods {
    /* all objects should have these implemented: */
    VAL                         (*get)                  (js_value_t*, js_string_t*);
    js_property_descriptor_t*   (*get_own_property)     (js_value_t*, js_string_t*);
    js_property_descriptor_t*   (*get_property)         (js_value_t*, js_string_t*);
    void                        (*put)                  (js_value_t*, js_string_t*, VAL);
    bool                        (*can_put)              (js_value_t*, js_string_t*);
    bool                        (*has_property)         (js_value_t*, js_string_t*);
    bool                        (*delete)               (js_value_t*, js_string_t*);
    VAL                         (*default_value)        (js_value_t*, js_type_t);
    bool                        (*define_own_property)  (js_value_t*, js_property_descriptor_t*);
} js_object_internal_methods_t;

VAL js_value_make_pointer(js_value_t* ptr);
VAL js_value_make_double(double num);
VAL js_value_make_string(char* buff, uint32_t len);
VAL js_value_make_cstring(char* str);
js_string_t* js_cstring(char* str);
VAL js_value_wrap_string(js_string_t* string);
VAL js_value_undefined();
VAL js_value_null();
VAL js_value_false();
VAL js_value_true();
VAL js_value_make_boolean(bool boolean);
VAL js_value_make_object(VAL prototype, VAL class);
VAL js_value_make_native_function(struct js_vm*, void* state, js_string_t* name, VAL(*call)(struct js_vm*, void*, VAL, uint32_t, VAL*), VAL(*construct)(struct js_vm*, void*, VAL, uint32_t, VAL*));
VAL js_value_make_function(struct js_vm* vm, struct js_image* image, uint32_t section, struct js_scope* outer_scope);

js_value_t* js_value_get_pointer(VAL val);
double js_value_get_double(VAL val);
bool js_value_is_truthy(VAL val);
bool js_value_is_object(VAL val);
bool js_value_is_primitive(VAL val);
js_type_t js_value_get_type(VAL val);

VAL js_to_object(struct js_vm* vm, VAL value);
VAL js_to_primitive(VAL value);
VAL js_to_boolean(VAL value);
VAL js_to_number(VAL value);
uint32_t js_to_uint32(VAL value);
int32_t js_to_int32(VAL value);
VAL js_to_string(VAL value);
js_string_t* js_to_js_string_t(VAL value);

VAL js_object_get(VAL obj, js_string_t* prop);
void js_object_put(VAL obj, js_string_t* prop, VAL value);
bool js_object_has_property(VAL obj, js_string_t* prop);
VAL js_object_default_value(VAL obj, js_type_t preferred_type);

VAL js_call(VAL fn, VAL this, uint32_t argc, VAL* argv);
VAL js_construct(VAL fn, uint32_t argc, VAL* argv);

#endif