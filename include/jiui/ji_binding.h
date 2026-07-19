/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_binding.h
 * @brief Data binding engine — connects data source properties to target
 *        object properties with configurable binding modes.
 *
 * Supports:
 *   - OneWay: source → target (default)
 *   - TwoWay: source ↔ target
 *   - OneTime: source → target once at bind time
 *   - OneWayToSource: target → source
 *
 * Data binding system for property synchronization.
 */

#ifndef JIUI_BINDING_H
#define JIUI_BINDING_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_property.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
typedef struct JiObject JiObject;
typedef struct JiBinding JiBinding;

/* =========================================================================
 * Binding mode
 * ========================================================================= */
typedef enum JiBindingMode {
    JI_BINDING_ONE_WAY           = 0,  /* source → target (default) */
    JI_BINDING_TWO_WAY           = 1,  /* source ↔ target */
    JI_BINDING_ONE_TIME          = 2,  /* source → target once */
    JI_BINDING_ONE_WAY_TO_SOURCE = 3   /* target → source */
} JiBindingMode;

/* =========================================================================
 * Binding — represents a single binding connection
 * ========================================================================= */
struct JiBinding {
    JiObject*       target;         /* Target object (bound to) */
    JiPropertyId    target_prop;    /* Target property ID */
    JiObject*       source;         /* Source object (bound from) */
    JiPropertyId    source_prop;    /* Source property ID */
    JiBindingMode   mode;           /* Binding mode */
    char*           source_path;    /* Source path (e.g. "UserName") for late binding */
    bool            is_active;       /* Whether the binding is currently active */
    int             source_sub_id;  /* Subscription ID for source changes */
    int             target_sub_id;  /* Subscription ID for target changes */
};

/* =========================================================================
 * Binding Engine — manages all active bindings
 * ========================================================================= */
typedef struct JiBindingEngine {
    JiBinding**     bindings;       /* Array of active bindings */
    int             count;          /* Number of active bindings */
    int             capacity;       /* Allocated capacity */
    bool            updating;       /* Reentrancy guard flag */
} JiBindingEngine;

/* =========================================================================
 * Binding API
 * ========================================================================= */

/**
 * Create a new binding between two properties.
 * @param target      Target object
 * @param target_prop Target property ID
 * @param source      Source object (or NULL for data context)
 * @param source_prop Source property ID
 * @param mode        Binding mode
 * @return New binding, or NULL on failure.
 */
JI_API JiBinding* ji_binding_new(JiObject* target, JiPropertyId target_prop,
                                   JiObject* source, JiPropertyId source_prop,
                                   JiBindingMode mode);

/**
 * Create a binding with a source path string (for late binding resolution).
 * The source property will be resolved at apply time from the data context.
 */
JI_API JiBinding* ji_binding_new_with_path(JiObject* target, JiPropertyId target_prop,
                                              const char* source_path,
                                              JiBindingMode mode);

/** Destroy a binding and unsubscribe from notifications. */
JI_API void ji_binding_destroy(JiBinding* binding);

/** Apply the binding: transfer the initial value from source to target. */
JI_API void ji_binding_apply(JiBinding* binding);

/** Detach the binding: unsubscribe from notifications. */
JI_API void ji_binding_detach(JiBinding* binding);

/** Check if a binding is active. */
JI_API bool ji_binding_is_active(const JiBinding* binding);

/** Get the binding mode as a string. */
JI_API const char* ji_binding_mode_str(JiBindingMode mode);

/** Parse a binding mode string (e.g. "OneWay", "TwoWay"). */
JI_API JiBindingMode ji_binding_mode_from_str(const char* str);

/* =========================================================================
 * Binding Engine API
 * ========================================================================= */

/** Initialize a binding engine. */
JI_API void ji_binding_engine_init(JiBindingEngine* engine);

/** Destroy a binding engine and all its bindings. */
JI_API void ji_binding_engine_destroy(JiBindingEngine* engine);

/**
 * Add a binding to the engine and apply it.
 * @return The binding, or NULL on failure.
 */
JI_API JiBinding* ji_binding_engine_add(JiBindingEngine* engine,
                                           JiObject* target, JiPropertyId target_prop,
                                           JiObject* source, JiPropertyId source_prop,
                                           JiBindingMode mode);

/**
 * Add a path-based binding to the engine and apply it.
 * The source will be resolved from the target's data context.
 */
JI_API JiBinding* ji_binding_engine_add_path(JiBindingEngine* engine,
                                                JiObject* target, JiPropertyId target_prop,
                                                const char* source_path,
                                                JiBindingMode mode);

/** Remove and destroy a binding from the engine. */
JI_API void ji_binding_engine_remove(JiBindingEngine* engine, JiBinding* binding);

/** Get the number of active bindings. */
JI_API int ji_binding_engine_count(const JiBindingEngine* engine);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_BINDING_H */
