/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_property.c
 * @brief Implementation of the property system — registration, value helpers.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_property.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <string.h>
#include <stdlib.h>

/* =========================================================================
 * Property registry — dynamic array
 * ========================================================================= */

#define JI_INITIAL_PROP_CAPACITY 128

static JiProperty* g_props       = NULL;
static int         g_prop_count  = 0;
static int         g_prop_capacity = 0;

static bool ji_property_ensure_capacity(void) {
    if (g_prop_count < g_prop_capacity) return true;
    int new_cap = g_prop_capacity == 0 ? JI_INITIAL_PROP_CAPACITY : g_prop_capacity * 2;
    JiProperty* new_arr = (JiProperty*)ji_realloc(g_props, (size_t)new_cap * sizeof(JiProperty));
    if (!new_arr) return false;
    g_props = new_arr;
    g_prop_capacity = new_cap;
    return true;
}

/* =========================================================================
 * Registration
 * ========================================================================= */

static JiPropertyId ji_property_register_internal(
    const char* name,
    JiTypeId owner_type,
    JiPropertyKind kind,
    JiPropertyType value_type,
    JiPropertyMetadata metadata)
{
    if (!name || owner_type < 0) {
        JI_WARN("ji_property_register: invalid args");
        return JI_PROPERTY_INVALID;
    }

    /* Check for duplicate name+owner */
    for (int i = 0; i < g_prop_count; i++) {
        if (g_props[i].owner_type == owner_type &&
            strcmp(g_props[i].name, name) == 0) {
            JI_WARN("ji_property_register: duplicate '%s' on type %d", name, owner_type);
            return JI_PROPERTY_INVALID;
        }
    }

    if (!ji_property_ensure_capacity()) {
        JI_ERROR_LOG("ji_property_register: out of memory");
        return JI_PROPERTY_INVALID;
    }

    JiPropertyId id = g_prop_count;
    JiProperty* p = &g_props[id];
    p->id         = id;
    p->name       = strdup(name);
    p->owner_type = owner_type;
    p->kind       = kind;
    p->value_type = value_type;
    p->metadata   = metadata;

    g_prop_count++;
    JI_TRACE("Registered property '%s' (id=%d, owner=%d, kind=%d, type=%d)",
             name, id, owner_type, kind, value_type);
    return id;
}

JiPropertyId ji_property_register_styled(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata)
{
    return ji_property_register_internal(name, owner_type, JI_PROP_STYLED, value_type, metadata);
}

JiPropertyId ji_property_register_direct(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata)
{
    return ji_property_register_internal(name, owner_type, JI_PROP_DIRECT, value_type, metadata);
}

JiPropertyId ji_property_register_attached(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata)
{
    return ji_property_register_internal(name, owner_type, JI_PROP_ATTACHED, value_type, metadata);
}

JiPropertyId ji_property_from_name(const char* name, JiTypeId owner_type) {
    if (!name) return JI_PROPERTY_INVALID;
    for (int i = 0; i < g_prop_count; i++) {
        if (g_props[i].owner_type == owner_type &&
            strcmp(g_props[i].name, name) == 0) {
            return i;
        }
    }
    return JI_PROPERTY_INVALID;
}

const JiProperty* ji_property_get(JiPropertyId id) {
    if (id < 0 || id >= g_prop_count) return NULL;
    return &g_props[id];
}

const char* ji_property_name(JiPropertyId id) {
    if (id < 0 || id >= g_prop_count) return NULL;
    return g_props[id].name;
}

JiPropertyType ji_property_value_type(JiPropertyId id) {
    if (id < 0 || id >= g_prop_count) return JI_PROP_TYPE_CUSTOM;
    return g_props[id].value_type;
}

JiPropertyKind ji_property_kind(JiPropertyId id) {
    if (id < 0 || id >= g_prop_count) return JI_PROP_STYLED;
    return g_props[id].kind;
}

bool ji_property_is_registered_on(JiPropertyId prop_id, JiTypeId type_id) {
    const JiProperty* p = ji_property_get(prop_id);
    if (!p) return false;
    /* Walk the type hierarchy to see if the owner type matches */
    return ji_type_is_a(type_id, p->owner_type);
}

/* =========================================================================
 * JiPropertyValue helpers
 * ========================================================================= */

JiPropertyValue ji_value_bool(bool v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type    = JI_PROP_TYPE_BOOL;
    val.v.bool_val = v;
    return val;
}

JiPropertyValue ji_value_int(int v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type    = JI_PROP_TYPE_INT;
    val.v.int_val = v;
    return val;
}

JiPropertyValue ji_value_float(float v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type      = JI_PROP_TYPE_FLOAT;
    val.v.float_val = v;
    return val;
}

JiPropertyValue ji_value_double(double v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type       = JI_PROP_TYPE_DOUBLE;
    val.v.double_val = v;
    return val;
}

JiPropertyValue ji_value_string(const char* v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type = JI_PROP_TYPE_STRING;
    if (v) {
        val.v.string_val = strdup(v);
    } else {
        val.v.string_val = NULL;
    }
    return val;
}

JiPropertyValue ji_value_ptr(void* v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type    = JI_PROP_TYPE_PTR;
    val.v.ptr_val = v;
    return val;
}

JiPropertyValue ji_value_object(JiObject* v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type       = JI_PROP_TYPE_OBJECT;
    val.v.object_val = v;
    return val;
}

JiPropertyValue ji_value_color(uint32_t argb) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type       = JI_PROP_TYPE_COLOR;
    val.v.color_val = argb;
    return val;
}

JiPropertyValue ji_value_int_enum(int v) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type    = JI_PROP_TYPE_ENUM;
    val.v.int_val = v;
    return val;
}

JiPropertyValue ji_value_null(JiPropertyType type) {
    JiPropertyValue val;
    memset(&val, 0, sizeof(val));
    val.type = type;
    return val;
}

void ji_value_destroy(JiPropertyValue* val) {
    if (!val) return;
    if (val->type == JI_PROP_TYPE_STRING && val->v.string_val) {
        ji_free(val->v.string_val);
        val->v.string_val = NULL;
    }
    val->type = JI_PROP_TYPE_CUSTOM;
}

JiPropertyValue ji_value_copy(const JiPropertyValue* val) {
    if (!val) return ji_value_null(JI_PROP_TYPE_CUSTOM);
    JiPropertyValue copy = *val;
    if (val->type == JI_PROP_TYPE_STRING && val->v.string_val) {
        copy.v.string_val = strdup(val->v.string_val);
    }
    return copy;
}

bool ji_value_equals(const JiPropertyValue* a, const JiPropertyValue* b) {
    if (!a || !b) return a == b;
    if (a->type != b->type) return false;

    switch (a->type) {
        case JI_PROP_TYPE_BOOL:   return a->v.bool_val   == b->v.bool_val;
        case JI_PROP_TYPE_INT:    return a->v.int_val    == b->v.int_val;
        case JI_PROP_TYPE_FLOAT:  return a->v.float_val  == b->v.float_val;
        case JI_PROP_TYPE_DOUBLE: return a->v.double_val == b->v.double_val;
        case JI_PROP_TYPE_STRING:
            if (a->v.string_val == b->v.string_val) return true;
            if (!a->v.string_val || !b->v.string_val) return false;
            return strcmp(a->v.string_val, b->v.string_val) == 0;
        case JI_PROP_TYPE_PTR:    return a->v.ptr_val    == b->v.ptr_val;
        case JI_PROP_TYPE_OBJECT: return a->v.object_val == b->v.object_val;
        case JI_PROP_TYPE_COLOR:  return a->v.color_val  == b->v.color_val;
        case JI_PROP_TYPE_ENUM:   return a->v.int_val    == b->v.int_val;
        default:                  return a->v.custom_val == b->v.custom_val;
    }
}
