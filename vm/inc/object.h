#ifndef JS_OBJECT_H
#define JS_OBJECT_H

#include "value.h"
#include "st.h"

int js_string_cmp(js_string_t* a, js_string_t* b);
st_table* js_st_table_new();
js_object_internal_methods_t* js_object_base_vtable();


#endif