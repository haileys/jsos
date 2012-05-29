#ifndef JS_SCOPE_H
#define JS_SCOPE_H

#include <stdbool.h>
#include "value.h"
#include "st.h"

typedef struct js_scope {
    /* if parent is NULL, this is a global scope. otherwise this is a local scope */
    struct js_scope* parent;
    struct js_scope* global;
    struct js_vm* vm;
    union {
        VAL global_object;
        struct {
            VAL callee;
            uint32_t count;
            VAL* vars;
        } locals;
    };
} js_scope_t;

js_scope_t* js_scope_make_global(struct js_vm* vm, VAL object);
VAL js_scope_get_var(js_scope_t* scope, uint32_t index, uint32_t upper_scopes);
void js_scope_set_var(js_scope_t* scope, uint32_t index, uint32_t upper_scopes, VAL value);
bool js_scope_has_var(js_scope_t* scope, uint32_t index, uint32_t upper_scopes);
js_scope_t* js_scope_close(js_scope_t* scope, VAL callee);
js_scope_t* js_scope_close_placement(js_scope_t* new_scope, js_scope_t* scope, VAL callee, uint32_t var_count, VAL* vars);

VAL js_scope_get_global_var(js_scope_t* scope, js_string_t* name);
void js_scope_set_global_var(js_scope_t* scope, js_string_t* name, VAL value);
bool js_scope_global_var_exists(js_scope_t* scope, js_string_t* name);
void js_scope_delete_global_var(js_scope_t* scope, js_string_t* name);

#endif