#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "st.h"
#include "gc.h"
#include "vm.h"
#include "exception.h"

int js_string_cmp(js_string_t* a, js_string_t* b)
{
    if(a->length < b->length) {
        return -1;
    }
    if(a->length > b->length) {
        return 1;
    }
    return memcmp(a->buff, b->buff, a->length);
}

static int js_string_hash(js_string_t* str)
{
    int val = 0;
    uint32_t i;
    for(i = 0; i < str->length; i++) {
        val += str->buff[i];
        val += (str->buff[i] << 10);
        val ^= (str->buff[i] >> 6);
    }
    val += (val << 3);
    val ^= (val >> 11);
    return val + (val << 15);
}

static struct st_hash_type js_string_st_type = {
    js_string_cmp,
    js_string_hash
};

st_table* js_st_table_new()
{
    return st_init_table(&js_string_st_type);
}

static VAL js_object_base_get(js_value_t* obj, js_string_t* prop)
{
    js_property_descriptor_t* descr = NULL;
    js_value_t* this = obj;
    while(!st_lookup(obj->object.properties, (st_data_t)prop, (st_data_t*)&descr)) {
        /* if not in object, look in prototype */
        if(js_value_is_primitive(obj->object.prototype)) {
            /* do not attempt if prototype is primitive */
            return js_value_undefined();
        }
        obj = js_value_get_pointer(obj->object.prototype);
    }
    if(!descr->is_accessor) {
        return descr->data.value;
    } else {
        if(js_value_get_type(descr->accessor.get) == JS_T_FUNCTION) {
            return js_call(descr->accessor.get, js_value_make_pointer(this), 0, NULL);
        }
        return js_value_undefined();
    }
}

static void js_object_base_put(js_value_t* obj, js_string_t* prop, VAL value)
{
    js_property_descriptor_t* descr = NULL;
    if(st_lookup(obj->object.properties, (st_data_t)prop, (st_data_t*)&descr)) {
        if(!descr->is_accessor) {
            if(descr->data.writable) {
                descr->data.value = value;
            }
        } else {
            if(js_value_get_type(descr->accessor.set) == JS_T_FUNCTION) {
                js_call(descr->accessor.set, js_value_make_pointer(obj), 1, &value);
            }
        }
        return;
    }
    descr = js_alloc(sizeof(js_property_descriptor_t));
    descr->is_accessor = false;
    descr->enumerable = true;
    descr->configurable = true;
    descr->data.value = value;
    descr->data.writable = true;
    st_insert(obj->object.properties, (st_data_t)prop, (st_data_t)descr);
}

static bool js_object_base_has_property(js_value_t* obj, js_string_t* prop)
{
    js_property_descriptor_t* descr = NULL;
    if(st_lookup(obj->object.properties, (st_data_t)prop, (st_data_t*)&descr)) {
        return true;
    }
    return false;
}

static VAL js_object_base_default_value(js_value_t* obj, js_type_t preferred_type)
{
    VAL fn, ret, this = js_value_make_pointer(obj);
    if(!preferred_type) {
        preferred_type = JS_T_NUMBER;
    }
    if(preferred_type == JS_T_STRING) {
        fn = js_object_get(this, js_cstring("toString"));
        if(js_value_get_type(fn) == JS_T_FUNCTION) {
            ret = js_call(fn, this, 0, NULL);
            if(js_value_is_primitive(ret)) {
                return ret;
            }
        }
        fn = js_object_get(this, js_cstring("valueOf"));
        if(js_value_get_type(fn) == JS_T_FUNCTION) {
            ret = js_call(fn, this, 0, NULL);
            if(js_value_is_primitive(ret)) {
                return ret;
            }
        }
        // @TODO throw exception
        js_panic("could not convert object to string");
    } else if(preferred_type == JS_T_NUMBER) {    
        fn = js_object_get(this, js_cstring("valueOf"));
        if(js_value_get_type(fn) == JS_T_FUNCTION) {
            ret = js_call(fn, this, 0, NULL);
            if(js_value_is_primitive(ret)) {
                return ret;
            }
        }
        fn = js_object_get(this, js_cstring("toString"));
        if(js_value_get_type(fn) == JS_T_FUNCTION) {
            ret = js_call(fn, this, 0, NULL);
            if(js_value_is_primitive(ret)) {
                return ret;
            }
        }
        // @TODO throw exception
        js_panic("could not convert object to string");
    }    
    js_panic("could not convert object to string");
}

static bool js_object_base_define_own_property(js_value_t* obj, js_string_t* prop, js_property_descriptor_t* new_descr)
{
    js_property_descriptor_t* old_descr = NULL;
    if(st_lookup(obj->object.properties, (st_data_t)prop, (st_data_t*)&old_descr)) {
        if(!old_descr->configurable) {
            return false;
        }
    }
    st_insert(obj->object.properties, (st_data_t)prop, (st_data_t)new_descr);
    return true;
}

static bool js_object_base_delete(js_value_t* obj, js_string_t* prop)
{
    st_data_t tmp;
    st_delete(obj->object.properties, (st_data_t*)&prop, &tmp);
    return true;
}

struct key_iter {
    js_string_t** keys;
    uint32_t index;
};

static int js_object_base_keys_iter(st_data_t key, st_data_t record, st_data_t arg)
{
    struct key_iter* state = (struct key_iter*)arg;
    state->keys[state->index] = (js_string_t*)key;
    return ST_CONTINUE;
}

static js_string_t** js_object_base_keys(js_value_t* obj, uint32_t* count)
{
    *count = obj->object.properties->num_entries;
    struct key_iter state = { js_alloc(sizeof(js_string_t*) * *count), 0 };
    st_foreach(obj->object.properties, js_object_base_keys_iter, (st_data_t)&state);
    return state.keys;
}

static js_object_internal_methods_t object_base_vtable = {
    /* get */                   js_object_base_get,
    /* get_own_property */      NULL,
    /* get_property */          NULL,
    /* put */                   js_object_base_put,
    /* can_put */               NULL,
    /* has_property */          js_object_base_has_property,
    /* delete */                js_object_base_delete,
    /* default_value */         js_object_base_default_value,
    /* define_own_property */   js_object_base_define_own_property,
    /* keys */                  js_object_base_keys,
    // @TODO: ^^ all those
};

js_object_internal_methods_t* js_object_base_vtable()
{
    return &object_base_vtable;
}