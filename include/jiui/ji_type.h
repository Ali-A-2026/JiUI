/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_type.h
 * @brief Runtime type descriptor system for JiUI objects.
 *
 * Every JiUI object type is described by a JiType descriptor that holds
 * the type name, size, parent type, and constructor/destructor functions.
 * This enables runtime type checking, safe downcasting, and iteration
 * over the type hierarchy.
 */

#ifndef JIUI_TYPE_H
#define JIUI_TYPE_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
typedef struct JiType       JiType;
typedef struct JiObject     JiObject;

/* =========================================================================
 * Type ID — unique integer identifying a registered type
 * ========================================================================= */
typedef int JiTypeId;

/** Invalid/uninitialized type ID. */
#define JI_TYPE_INVALID ((JiTypeId)-1)

/** Base type ID for JiObject itself. */
#define JI_TYPE_OBJECT   ((JiTypeId)0)

/* =========================================================================
 * Constructor / Destructor function types
 * ========================================================================= */

/** Called after a JiObject is allocated and zeroed. Returns 0 on success. */
typedef int (*JiConstructorFunc)(JiObject* obj);

/** Called before a JiObject is freed. */
typedef void (*JiDestructorFunc)(JiObject* obj);

/* =========================================================================
 * JiType — runtime type descriptor
 * ========================================================================= */
struct JiType {
    JiTypeId           id;           /* unique type id                    */
    const char*        name;         /* human-readable type name           */
    size_t             instance_size;/* sizeof the full struct             */
    JiTypeId           parent_id;    /* parent type id (JI_TYPE_INVALID if root) */
    JiConstructorFunc  construct;    /* optional constructor               */
    JiDestructorFunc   destruct;     /* optional destructor                */
};

/* =========================================================================
 * Type registration & query API
 * ========================================================================= */

/**
 * Register a new type with the runtime.
 * @param name         Type name (must be unique, not NULL)
 * @param instance_size Size of the struct (must be >= sizeof(JiObject))
 * @param parent_id    Parent type ID (JI_TYPE_INVALID for no parent)
 * @param construct    Optional constructor
 * @param destruct     Optional destructor
 * @return The new type ID, or JI_TYPE_INVALID on failure.
 */
JI_API JiTypeId ji_type_register(const char* name,
                                  size_t instance_size,
                                  JiTypeId parent_id,
                                  JiConstructorFunc construct,
                                  JiDestructorFunc destruct);

/**
 * Look up a type by name.
 * @return The type ID, or JI_TYPE_INVALID if not found.
 */
JI_API JiTypeId ji_type_from_name(const char* name);

/**
 * Get the type descriptor for a type ID.
 * @return Pointer to the type descriptor, or NULL if invalid.
 */
JI_API const JiType* ji_type_get(JiTypeId id);

/**
 * Check if a type is derived from (or is) another type.
 */
JI_API bool ji_type_is_a(JiTypeId type_id, JiTypeId base_id);

/**
 * Get the parent type ID.
 * @return Parent type ID, or JI_TYPE_INVALID if the type has no parent.
 */
JI_API JiTypeId ji_type_parent(JiTypeId id);

/**
 * Get the type name.
 * @return Type name string, or NULL if invalid.
 */
JI_API const char* ji_type_name(JiTypeId id);

/**
 * Get the number of registered types.
 */
JI_API int ji_type_count(void);

/**
 * Initialize the type system (registers the base JiObject type).
 * Called automatically by ji_initialize().
 */
JI_API void ji_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TYPE_H */
