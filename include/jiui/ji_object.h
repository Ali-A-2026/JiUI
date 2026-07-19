/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_object.h
 * @brief JiObject — the base class for all JiUI objects with property store,
 *        change notifications, and data context support.
 */

#ifndef JIUI_OBJECT_H
#define JIUI_OBJECT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_error.h"
#include "ji_type.h"
#include "ji_property.h"
#include "ji_memory.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Property store entry — maps PropertyId → Value
 * ========================================================================= */
typedef struct JiPropertyEntry {
    JiPropertyId   id;
    JiPropertyValue value;
    bool           is_set;       /* true if explicitly set (not default) */
} JiPropertyEntry;

/* =========================================================================
 * JiObject — base class for all JiUI objects
 *
 * Subclasses embed JiObject as their first member and register their
 * own JiType with a larger instance_size.
 * ========================================================================= */
struct JiObject {
    JiRefObject         base;          /* ref-counted base (MUST be first) */
    JiTypeId            type_id;       /* runtime type                    */
    JiPropertyEntry*    properties;     /* dynamic array of set properties  */
    int                 prop_count;     /* number of set properties         */
    int                 prop_capacity;  /* allocated capacity                */
    JiObject*           parent;         /* logical parent (for inheritance) */
    JiObject**          children;       /* logical children array            */
    int                 child_count;    /* number of logical children       */
    int                 child_capacity; /* allocated capacity for children  */
    void*               data_context;   /* bound data context               */
    char*               name;           /* optional element name (#name)     */
};

/* =========================================================================
 * Object lifecycle
 * ========================================================================= */

/**
 * Allocate and initialize a new JiObject (or subclass).
 * The type_id determines the actual allocation size.
 * @param type_id The type to instantiate.
 * @return New object with ref count = 1, or NULL on failure.
 */
JI_API JiObject* ji_object_new(JiTypeId type_id);

/**
 * Create a JiObject of the base JiObject type.
 */
JI_API JiObject* ji_object_create(void);

/**
 * Initialize a pre-allocated JiObject (for stack or embedded use).
 */
JI_API JiResultCode ji_object_init(JiObject* obj, JiTypeId type_id);

/**
 * Destroy a JiObject (called automatically when ref count reaches 0).
 */
JI_API void ji_object_destroy(JiObject* obj);

/* =========================================================================
 * Type checking
 * ========================================================================= */

/** Get the type ID of an object. */
JI_API JiTypeId ji_object_type_id(const JiObject* obj);

/** Get the type descriptor of an object. */
JI_API const JiType* ji_object_type(const JiObject* obj);

/** Check if an object is of a given type (or derived from it). */
JI_API bool ji_object_is_a(const JiObject* obj, JiTypeId type_id);

/** Get the object's name (or NULL). */
JI_API const char* ji_object_name(const JiObject* obj);

/** Set the object's name. */
JI_API void ji_object_set_name(JiObject* obj, const char* name);

/* =========================================================================
 * Property get / set
 * ========================================================================= */

/**
 * Get a property value.
 * If the property is not explicitly set, returns the default from metadata.
 * @return JI_OK on success, error code on failure.
 */
JI_API JiResultCode ji_object_get_property(JiObject* obj,
                                            JiPropertyId prop_id,
                                            JiPropertyValue* out_val);

/**
 * Set a property value.
 * Triggers coercion, validation, and change notification.
 * @return JI_OK on success, error code on failure.
 */
JI_API JiResultCode ji_object_set_property(JiObject* obj,
                                             JiPropertyId prop_id,
                                             JiPropertyValue value);

/**
 * Clear a property value (revert to default).
 * Triggers change notification if the value was previously set.
 */
JI_API JiResultCode ji_object_clear_property(JiObject* obj, JiPropertyId prop_id);

/**
 * Check if a property is explicitly set on this object.
 */
JI_API bool ji_object_is_property_set(const JiObject* obj, JiPropertyId prop_id);

/* =========================================================================
 * Convenience typed property accessors
 * ========================================================================= */

JI_API bool         ji_object_get_bool(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_bool(JiObject* obj, JiPropertyId prop_id, bool val);

JI_API int          ji_object_get_int(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_int(JiObject* obj, JiPropertyId prop_id, int val);

JI_API double       ji_object_get_double(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_double(JiObject* obj, JiPropertyId prop_id, double val);

JI_API const char*  ji_object_get_string(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_string(JiObject* obj, JiPropertyId prop_id, const char* val);

JI_API void*        ji_object_get_ptr(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_ptr(JiObject* obj, JiPropertyId prop_id, void* val);

JI_API uint32_t     ji_object_get_color(JiObject* obj, JiPropertyId prop_id);
JI_API JiResultCode ji_object_set_color(JiObject* obj, JiPropertyId prop_id, uint32_t argb);

/* =========================================================================
 * Change notification
 * ========================================================================= */

/** Callback for property changes on a specific object. */
typedef void (*JiObjectPropertyChangedFunc)(JiObject* obj,
                                             JiPropertyId prop_id,
                                             const JiPropertyValue* old_val,
                                             const JiPropertyValue* new_val,
                                             void* user_data);

/** Subscribe to property changes on an object. Returns a subscription ID (>=0) or -1 on error. */
JI_API int  ji_object_add_property_changed(JiObject* obj,
                                             JiObjectPropertyChangedFunc callback,
                                             void* user_data);

/** Remove a subscription by ID. */
JI_API void ji_object_remove_property_changed(JiObject* obj, int subscription_id);

/* =========================================================================
 * Data context
 * ========================================================================= */

/** Set the data context for data binding. */
JI_API void ji_object_set_data_context(JiObject* obj, void* context);

/** Get the data context. */
JI_API void* ji_object_get_data_context(const JiObject* obj);

/* =========================================================================
 * Logical tree
 * ========================================================================= */

/** Set the logical parent of this object and register as a child. */
JI_API void ji_object_set_parent(JiObject* obj, JiObject* parent);

/** Get the number of logical children. */
JI_API int ji_object_get_child_count(const JiObject* obj);

/** Get a logical child by index (0-based). Returns NULL if out of range. */
JI_API JiObject* ji_object_get_child(const JiObject* obj, int index);

/** Add a child object. Also sets the child's parent to this object. */
JI_API void ji_object_add_child(JiObject* parent, JiObject* child);

/** Get the logical parent. */
JI_API JiObject* ji_object_get_parent(const JiObject* obj);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_OBJECT_H */
