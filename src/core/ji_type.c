/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_type.c
 * @brief Implementation of the runtime type descriptor system.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_type.h"
#include "jiui/ji_object.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <string.h>
#include <stdlib.h>

/* =========================================================================
 * Type registry — simple dynamic array
 * ========================================================================= */

#define JI_INITIAL_TYPE_CAPACITY 64

static JiType*  g_types     = NULL;
static int      g_type_count = 0;
static int      g_type_capacity = 0;

/* =========================================================================
 * Internal: ensure capacity
 * ========================================================================= */
static bool ji_type_ensure_capacity(void) {
    if (g_type_count < g_type_capacity) return true;
    int new_cap = g_type_capacity == 0 ? JI_INITIAL_TYPE_CAPACITY : g_type_capacity * 2;
    JiType* new_arr = (JiType*)ji_realloc(g_types, (size_t)new_cap * sizeof(JiType));
    if (!new_arr) return false;
    g_types = new_arr;
    g_type_capacity = new_cap;
    return true;
}

/* =========================================================================
 * Internal: register the base JiObject type (id=0)
 * ========================================================================= */
static bool g_type_initialized = false;

static void ji_type_register_base(void) {
    if (g_type_count > 0) return; /* already registered */
    ji_type_ensure_capacity();
    JiType* t = &g_types[0];
    t->id            = JI_TYPE_OBJECT;
    t->name          = "JiObject";
    t->instance_size = sizeof(JiObject);
    t->parent_id     = JI_TYPE_INVALID;
    t->construct     = NULL;
    t->destruct      = NULL;
    g_type_count     = 1;
    g_type_initialized = true;
}

void ji_type_init(void) {
    ji_type_register_base();
}

/* =========================================================================
 * API implementation
 * ========================================================================= */

JiTypeId ji_type_register(const char* name,
                           size_t instance_size,
                           JiTypeId parent_id,
                           JiConstructorFunc construct,
                           JiDestructorFunc destruct) {
    if (!name || instance_size < sizeof(JiObject)) {
        JI_WARN("ji_type_register: invalid args (name=%p size=%zu)", name, instance_size);
        return JI_TYPE_INVALID;
    }

    /* Ensure base type is registered */
    ji_type_register_base();

    /* Validate parent type if specified */
    if (parent_id != JI_TYPE_INVALID) {
        if (parent_id < 0 || parent_id >= g_type_count) {
            JI_WARN("ji_type_register: invalid parent_id %d", parent_id);
            return JI_TYPE_INVALID;
        }
    }

    /* Check for duplicate name */
    for (int i = 0; i < g_type_count; i++) {
        if (strcmp(g_types[i].name, name) == 0) {
            JI_WARN("ji_type_register: duplicate type name '%s'", name);
            return JI_TYPE_INVALID;
        }
    }

    if (!ji_type_ensure_capacity()) {
        JI_ERROR_LOG("ji_type_register: out of memory");
        return JI_TYPE_INVALID;
    }

    JiTypeId id = g_type_count;
    JiType* t = &g_types[id];
    t->id            = id;
    t->name          = strdup(name);
    t->instance_size = instance_size;
    t->parent_id     = parent_id;
    t->construct     = construct;
    t->destruct      = destruct;

    g_type_count++;
    JI_TRACE("Registered type '%s' (id=%d, parent=%d, size=%zu)",
             name, id, parent_id, instance_size);
    return id;
}

JiTypeId ji_type_from_name(const char* name) {
    if (!name) return JI_TYPE_INVALID;
    for (int i = 0; i < g_type_count; i++) {
        if (strcmp(g_types[i].name, name) == 0) return i;
    }
    return JI_TYPE_INVALID;
}

const JiType* ji_type_get(JiTypeId id) {
    if (id < 0 || id >= g_type_count) return NULL;
    return &g_types[id];
}

bool ji_type_is_a(JiTypeId type_id, JiTypeId base_id) {
    if (type_id < 0 || type_id >= g_type_count) return false;
    if (base_id < 0 || base_id >= g_type_count) return false;

    JiTypeId current = type_id;
    while (current != JI_TYPE_INVALID) {
        if (current == base_id) return true;
        if (current >= g_type_count) return false;
        current = g_types[current].parent_id;
    }
    return false;
}

JiTypeId ji_type_parent(JiTypeId id) {
    if (id < 0 || id >= g_type_count) return JI_TYPE_INVALID;
    return g_types[id].parent_id;
}

const char* ji_type_name(JiTypeId id) {
    if (id < 0 || id >= g_type_count) return NULL;
    return g_types[id].name;
}

int ji_type_count(void) {
    return g_type_count;
}
