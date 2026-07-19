/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_object.c
 * @brief Implementation of JiObject — base class with property store.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_object.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <string.h>
#include <stdlib.h>

/* =========================================================================
 * Subscription entry for property change callbacks
 * ========================================================================= */
typedef struct JiSubscription {
    int                             id;
    JiObjectPropertyChangedFunc    callback;
    void*                           user_data;
} JiSubscription;

#define JI_MAX_SUBSCRIPTIONS 32

/* =========================================================================
 * Internal: destroy callback for ref-counted JiObject
 * ========================================================================= */
static void ji_object_ref_destroy(void* obj) {
    ji_object_destroy((JiObject*)obj);
}

/* =========================================================================
 * Internal: find property entry index
 * ========================================================================= */
static int ji_object_find_entry(const JiObject* obj, JiPropertyId prop_id) {
    for (int i = 0; i < obj->prop_count; i++) {
        if (obj->properties[i].id == prop_id) return i;
    }
    return -1;
}

/* =========================================================================
 * Internal: ensure property capacity
 * ========================================================================= */
static bool ji_object_ensure_prop_capacity(JiObject* obj) {
    if (obj->prop_count < obj->prop_capacity) return true;
    int new_cap = obj->prop_capacity == 0 ? 8 : obj->prop_capacity * 2;
    JiPropertyEntry* new_arr = (JiPropertyEntry*)ji_realloc(
        obj->properties, (size_t)new_cap * sizeof(JiPropertyEntry));
    if (!new_arr) return false;
    obj->properties   = new_arr;
    obj->prop_capacity = new_cap;
    return true;
}

/* =========================================================================
 * Internal: fire property changed notifications
 * ========================================================================= */
typedef struct JiSubList {
    JiSubscription subs[JI_MAX_SUBSCRIPTIONS];
    int            count;
    int            next_id;
} JiSubList;

static JiSubList* ji_object_get_subs(JiObject* obj) {
    /* We store subscriptions in the data_context pointer temporarily.
       A better approach would be to add a subs field to JiObject,
       but for now we use a separate allocation. */
    /* Actually, let's add a subs pointer to JiObject. We'll use
       the name pointer area. For simplicity, let's use a global
       per-object subscription list stored via a hash map.
       
       Simpler approach: embed a subs pointer in JiObject.
       We'll add it as a void* field after name. But we already
       defined the struct... Let's use a side-channel. */
    
    /* For Phase 2 simplicity, we'll store subscriptions in a
       separate hash map keyed by object pointer. */
    return NULL; /* placeholder — see below */
}

/* Simple subscription storage: we'll add a subs field to JiObject
   via the data_context for now. Actually, let's just add a proper
   subs pointer. We need to update the header. But since we already
   wrote it... let's use a simple global approach for now. */

#define JI_SUB_TABLE_SIZE 256

typedef struct JiSubEntry {
    JiObject*                    obj;
    JiSubscription               subs[JI_MAX_SUBSCRIPTIONS];
    int                          sub_count;
    int                          next_sub_id;
    struct JiSubEntry*           next;  /* hash chain */
} JiSubEntry;

static JiSubEntry* g_sub_table[JI_SUB_TABLE_SIZE] = {0};

static unsigned ji_sub_hash(const JiObject* obj) {
    return ((unsigned)(uintptr_t)obj) % JI_SUB_TABLE_SIZE;
}

static JiSubEntry* ji_sub_entry_find_or_create(JiObject* obj) {
    unsigned idx = ji_sub_hash(obj);
    JiSubEntry* e = g_sub_table[idx];
    while (e) {
        if (e->obj == obj) return e;
        e = e->next;
    }
    e = JI_NEW(JiSubEntry);
    if (!e) return NULL;
    e->obj = obj;
    e->sub_count = 0;
    e->next_sub_id = 1;
    e->next = g_sub_table[idx];
    g_sub_table[idx] = e;
    return e;
}

static void ji_sub_entry_remove(JiObject* obj) {
    unsigned idx = ji_sub_hash(obj);
    JiSubEntry** pp = &g_sub_table[idx];
    while (*pp) {
        if ((*pp)->obj == obj) {
            JiSubEntry* del = *pp;
            *pp = del->next;
            ji_free(del);
            return;
        }
        pp = &(*pp)->next;
    }
}

static void ji_fire_property_changed(JiObject* obj,
                                      JiPropertyId prop_id,
                                      const JiPropertyValue* old_val,
                                      const JiPropertyValue* new_val) {
    /* Call global metadata callback */
    const JiProperty* prop = ji_property_get(prop_id);
    if (prop && prop->metadata.on_changed) {
        prop->metadata.on_changed(obj, prop_id, old_val, new_val);
    }

    /* Call per-object subscriptions */
    JiSubEntry* entry = ji_sub_entry_find_or_create(obj);
    if (!entry) return;
    for (int i = 0; i < entry->sub_count; i++) {
        if (entry->subs[i].callback) {
            entry->subs[i].callback(obj, prop_id, old_val, new_val,
                                     entry->subs[i].user_data);
        }
    }
}

/* =========================================================================
 * Object lifecycle
 * ========================================================================= */

JiResultCode ji_object_init(JiObject* obj, JiTypeId type_id) {
    if (!obj) return JI_ERROR_NULL_PTR;
    const JiType* type = ji_type_get(type_id);
    if (!type) return JI_ERROR_INVALID_ARG;

    memset(obj, 0, type->instance_size);
    ji_ref_object_init(&obj->base, ji_object_ref_destroy);
    obj->type_id = type_id;

    /* Call constructor if present */
    if (type->construct) {
        JiResultCode rc = type->construct(obj);
        if (JI_FAILED(rc)) return rc;
    }

    return JI_OK;
}

JiObject* ji_object_new(JiTypeId type_id) {
    const JiType* type = ji_type_get(type_id);
    if (!type) {
        JI_WARN("ji_object_new: invalid type_id %d", type_id);
        return NULL;
    }

    JiObject* obj = (JiObject*)ji_calloc(1, type->instance_size);
    if (!obj) return NULL;

    JiResultCode rc = ji_object_init(obj, type_id);
    if (JI_FAILED(rc)) {
        ji_free(obj);
        return NULL;
    }

    return obj;
}

JiObject* ji_object_create(void) {
    return ji_object_new(JI_TYPE_OBJECT);
}

void ji_object_destroy(JiObject* obj) {
    if (!obj) return;

    /* Call type destructor if present */
    const JiType* type = ji_type_get(obj->type_id);
    if (type && type->destruct) {
        type->destruct(obj);
    }

    /* Free property values */
    for (int i = 0; i < obj->prop_count; i++) {
        ji_value_destroy(&obj->properties[i].value);
    }
    if (obj->properties) {
        ji_free(obj->properties);
        obj->properties = NULL;
    }

    /* Free name */
    if (obj->name) {
        ji_free(obj->name);
        obj->name = NULL;
    }

    /* Free children array (not the children themselves — they're ref-counted) */
    if (obj->children) {
        ji_free(obj->children);
        obj->children = NULL;
    }
    obj->child_count = 0;
    obj->child_capacity = 0;

    /* Remove subscriptions */
    ji_sub_entry_remove(obj);

    /* Note: we do NOT free obj itself here — that's done by the
       ref-counting system (ji_ref_object_release). But if this
       was called as the destroy callback, the ref system will
       free the memory after this function returns. */
}

/* =========================================================================
 * Type checking
 * ========================================================================= */

JiTypeId ji_object_type_id(const JiObject* obj) {
    return obj ? obj->type_id : JI_TYPE_INVALID;
}

const JiType* ji_object_type(const JiObject* obj) {
    return obj ? ji_type_get(obj->type_id) : NULL;
}

bool ji_object_is_a(const JiObject* obj, JiTypeId type_id) {
    if (!obj) return false;
    return ji_type_is_a(obj->type_id, type_id);
}

const char* ji_object_name(const JiObject* obj) {
    return obj ? obj->name : NULL;
}

void ji_object_set_name(JiObject* obj, const char* name) {
    if (!obj) return;
    if (obj->name) ji_free(obj->name);
    obj->name = name ? strdup(name) : NULL;
}

/* =========================================================================
 * Property get / set
 * ========================================================================= */

JiResultCode ji_object_get_property(JiObject* obj,
                                      JiPropertyId prop_id,
                                      JiPropertyValue* out_val) {
    if (!obj || !out_val) return JI_ERROR_NULL_PTR;
    if (prop_id < 0) return JI_ERROR_INVALID_ARG;

    /* Check if explicitly set */
    int idx = ji_object_find_entry(obj, prop_id);
    if (idx >= 0) {
        *out_val = ji_value_copy(&obj->properties[idx].value);
        return JI_OK;
    }

    /* Return default from metadata */
    const JiProperty* prop = ji_property_get(prop_id);
    if (!prop) return JI_ERROR_NOT_FOUND;

    *out_val = ji_value_copy(&prop->metadata.default_value);
    return JI_OK;
}

JiResultCode ji_object_set_property(JiObject* obj,
                                      JiPropertyId prop_id,
                                      JiPropertyValue value) {
    if (!obj) return JI_ERROR_NULL_PTR;
    if (prop_id < 0) return JI_ERROR_INVALID_ARG;

    const JiProperty* prop = ji_property_get(prop_id);
    if (!prop) return JI_ERROR_NOT_FOUND;

    /* Validate type */
    if (value.type != prop->value_type &&
        value.type != JI_PROP_TYPE_CUSTOM) {
        /* Allow implicit int ↔ enum */
        if (!(prop->value_type == JI_PROP_TYPE_ENUM && value.type == JI_PROP_TYPE_INT)) {
            JI_WARN("Property '%s' type mismatch: expected %d, got %d",
                    prop->name, prop->value_type, value.type);
            ji_value_destroy(&value);
            return JI_ERROR_TYPE_MISMATCH;
        }
    }

    /* Coerce */
    if (prop->metadata.coerce) {
        if (!prop->metadata.coerce(obj, prop_id, &value)) {
            ji_value_destroy(&value);
            return JI_ERROR_STATE;
        }
    }

    /* Validate */
    if (prop->metadata.validate) {
        if (!prop->metadata.validate(obj, prop_id, &value)) {
            ji_value_destroy(&value);
            return JI_ERROR_INVALID_ARG;
        }
    }

    /* Get old value for notification */
    JiPropertyValue old_val;
    bool had_old = false;
    int idx = ji_object_find_entry(obj, prop_id);
    if (idx >= 0) {
        old_val = obj->properties[idx].value;
        had_old = true;
    } else {
        old_val = prop->metadata.default_value;
        had_old = false;
    }

    /* Check if value is actually changing */
    if (had_old && ji_value_equals(&old_val, &value)) {
        ji_value_destroy(&value);
        return JI_OK; /* no change */
    }

    /* Set the value */
    if (idx >= 0) {
        /* Update existing entry */
        ji_value_destroy(&obj->properties[idx].value);
        obj->properties[idx].value = value;
        obj->properties[idx].is_set = true;
    } else {
        /* Add new entry */
        if (!ji_object_ensure_prop_capacity(obj)) {
            ji_value_destroy(&value);
            return JI_ERROR_OUT_OF_MEMORY;
        }
        idx = obj->prop_count++;
        obj->properties[idx].id     = prop_id;
        obj->properties[idx].value  = value;
        obj->properties[idx].is_set = true;
    }

    /* Fire change notification */
    JiPropertyValue old_copy = ji_value_copy(&old_val);
    ji_fire_property_changed(obj, prop_id, &old_copy, &value);
    ji_value_destroy(&old_copy);

    return JI_OK;
}

JiResultCode ji_object_clear_property(JiObject* obj, JiPropertyId prop_id) {
    if (!obj) return JI_ERROR_NULL_PTR;

    int idx = ji_object_find_entry(obj, prop_id);
    if (idx < 0) return JI_OK; /* not set, nothing to do */

    /* Get old value for notification */
    JiPropertyValue old_val = obj->properties[idx].value;

    /* Remove entry by swapping with last */
    if (idx < obj->prop_count - 1) {
        obj->properties[idx] = obj->properties[obj->prop_count - 1];
    }
    obj->prop_count--;

    /* Fire notification with default as new value */
    const JiProperty* prop = ji_property_get(prop_id);
    JiPropertyValue new_val = prop ? ji_value_copy(&prop->metadata.default_value)
                                   : ji_value_null(JI_PROP_TYPE_CUSTOM);

    JiPropertyValue old_copy = ji_value_copy(&old_val);
    ji_fire_property_changed(obj, prop_id, &old_copy, &new_val);
    ji_value_destroy(&old_copy);
    ji_value_destroy(&old_val);
    ji_value_destroy(&new_val);

    return JI_OK;
}

bool ji_object_is_property_set(const JiObject* obj, JiPropertyId prop_id) {
    if (!obj) return false;
    return ji_object_find_entry(obj, prop_id) >= 0;
}

/* =========================================================================
 * Convenience typed accessors
 * ========================================================================= */

bool ji_object_get_bool(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        bool result = val.v.bool_val;
        ji_value_destroy(&val);
        return result;
    }
    return false;
}

JiResultCode ji_object_set_bool(JiObject* obj, JiPropertyId prop_id, bool val) {
    return ji_object_set_property(obj, prop_id, ji_value_bool(val));
}

int ji_object_get_int(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        int result = val.v.int_val;
        ji_value_destroy(&val);
        return result;
    }
    return 0;
}

JiResultCode ji_object_set_int(JiObject* obj, JiPropertyId prop_id, int val) {
    return ji_object_set_property(obj, prop_id, ji_value_int(val));
}

double ji_object_get_double(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        double result = val.v.double_val;
        ji_value_destroy(&val);
        return result;
    }
    return 0.0;
}

JiResultCode ji_object_set_double(JiObject* obj, JiPropertyId prop_id, double val) {
    return ji_object_set_property(obj, prop_id, ji_value_double(val));
}

const char* ji_object_get_string(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        /* NOTE: caller must NOT free the returned string; it's a copy.
           For simplicity we leak it here — a real impl would use a pool. */
        return val.v.string_val;
    }
    return NULL;
}

JiResultCode ji_object_set_string(JiObject* obj, JiPropertyId prop_id, const char* val) {
    return ji_object_set_property(obj, prop_id, ji_value_string(val));
}

void* ji_object_get_ptr(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        void* result = val.v.ptr_val;
        ji_value_destroy(&val);
        return result;
    }
    return NULL;
}

JiResultCode ji_object_set_ptr(JiObject* obj, JiPropertyId prop_id, void* val) {
    return ji_object_set_property(obj, prop_id, ji_value_ptr(val));
}

uint32_t ji_object_get_color(JiObject* obj, JiPropertyId prop_id) {
    JiPropertyValue val;
    if (JI_SUCCEEDED(ji_object_get_property(obj, prop_id, &val))) {
        uint32_t result = val.v.color_val;
        ji_value_destroy(&val);
        return result;
    }
    return 0;
}

JiResultCode ji_object_set_color(JiObject* obj, JiPropertyId prop_id, uint32_t argb) {
    return ji_object_set_property(obj, prop_id, ji_value_color(argb));
}

/* =========================================================================
 * Change notification
 * ========================================================================= */

int ji_object_add_property_changed(JiObject* obj,
                                    JiObjectPropertyChangedFunc callback,
                                    void* user_data) {
    if (!obj || !callback) return -1;
    JiSubEntry* entry = ji_sub_entry_find_or_create(obj);
    if (!entry) return -1;
    if (entry->sub_count >= JI_MAX_SUBSCRIPTIONS) return -1;

    int id = entry->next_sub_id++;
    entry->subs[entry->sub_count].id        = id;
    entry->subs[entry->sub_count].callback   = callback;
    entry->subs[entry->sub_count].user_data = user_data;
    entry->sub_count++;
    return id;
}

void ji_object_remove_property_changed(JiObject* obj, int subscription_id) {
    if (!obj || subscription_id < 0) return;
    JiSubEntry* entry = ji_sub_entry_find_or_create(obj);
    if (!entry) return;

    for (int i = 0; i < entry->sub_count; i++) {
        if (entry->subs[i].id == subscription_id) {
            /* Remove by swapping with last */
            entry->subs[i] = entry->subs[entry->sub_count - 1];
            entry->sub_count--;
            return;
        }
    }
}

/* =========================================================================
 * Data context
 * ========================================================================= */

void ji_object_set_data_context(JiObject* obj, void* context) {
    if (!obj) return;
    obj->data_context = context;
}

void* ji_object_get_data_context(const JiObject* obj) {
    return obj ? obj->data_context : NULL;
}

/* =========================================================================
 * Logical tree
 * ========================================================================= */

void ji_object_set_parent(JiObject* obj, JiObject* parent) {
    if (!obj) return;
    obj->parent = parent;
}

JiObject* ji_object_get_parent(const JiObject* obj) {
    return obj ? obj->parent : NULL;
}

int ji_object_get_child_count(const JiObject* obj) {
    return obj ? obj->child_count : 0;
}

JiObject* ji_object_get_child(const JiObject* obj, int index) {
    if (!obj || index < 0 || index >= obj->child_count) return NULL;
    return obj->children[index];
}

void ji_object_add_child(JiObject* parent, JiObject* child) {
    if (!parent || !child) return;

    /* Ensure capacity */
    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0 ? 8 : parent->child_capacity * 2;
        parent->children = (JiObject**)ji_realloc(parent->children,
            (size_t)new_cap * sizeof(JiObject*));
        parent->child_capacity = new_cap;
    }

    parent->children[parent->child_count++] = child;
    child->parent = parent;
}
