/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_binding.c
 * @brief Implementation of the data binding engine.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_binding.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_object.h"
#include "jiui/ji_property.h"

#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Internal: property change callback for source → target propagation
 * ========================================================================= */
static void binding_source_changed(JiObject* obj, JiPropertyId prop_id,
                                     const JiPropertyValue* old_val,
                                     const JiPropertyValue* new_val,
                                     void* user_data) {
    (void)obj; (void)old_val;
    JiBinding* binding = (JiBinding*)user_data;
    if (!binding || !binding->is_active) return;

    if (prop_id != binding->source_prop) return;

    /* OneWay or TwoWay: propagate source → target */
    if (binding->mode == JI_BINDING_ONE_WAY || binding->mode == JI_BINDING_TWO_WAY) {
        if (binding->target && new_val) {
            ji_object_set_property(binding->target, binding->target_prop, *new_val);
        }
    }
}

/* =========================================================================
 * Internal: property change callback for target → source propagation
 * ========================================================================= */
static void binding_target_changed(JiObject* obj, JiPropertyId prop_id,
                                     const JiPropertyValue* old_val,
                                     const JiPropertyValue* new_val,
                                     void* user_data) {
    (void)obj; (void)old_val;
    JiBinding* binding = (JiBinding*)user_data;
    if (!binding || !binding->is_active) return;

    if (prop_id != binding->target_prop) return;

    /* TwoWay or OneWayToSource: propagate target → source */
    if (binding->mode == JI_BINDING_TWO_WAY || binding->mode == JI_BINDING_ONE_WAY_TO_SOURCE) {
        if (binding->source && new_val) {
            ji_object_set_property(binding->source, binding->source_prop, *new_val);
        }
    }
}

/* =========================================================================
 * Binding lifecycle
 * ========================================================================= */

JI_API JiBinding* ji_binding_new(JiObject* target, JiPropertyId target_prop,
                                   JiObject* source, JiPropertyId source_prop,
                                   JiBindingMode mode) {
    JiBinding* binding = JI_NEW(JiBinding);
    if (!binding) return NULL;
    memset(binding, 0, sizeof(JiBinding));

    binding->target = target;
    binding->target_prop = target_prop;
    binding->source = source;
    binding->source_prop = source_prop;
    binding->mode = mode;
    binding->source_path = NULL;
    binding->is_active = false;
    binding->source_sub_id = -1;
    binding->target_sub_id = -1;

    return binding;
}

JI_API JiBinding* ji_binding_new_with_path(JiObject* target, JiPropertyId target_prop,
                                              const char* source_path,
                                              JiBindingMode mode) {
    JiBinding* binding = JI_NEW(JiBinding);
    if (!binding) return NULL;
    memset(binding, 0, sizeof(JiBinding));

    binding->target = target;
    binding->target_prop = target_prop;
    binding->source = NULL;
    binding->source_prop = JI_PROPERTY_INVALID;
    binding->mode = mode;
    binding->source_path = source_path ? strdup(source_path) : NULL;
    binding->is_active = false;
    binding->source_sub_id = -1;
    binding->target_sub_id = -1;

    return binding;
}

JI_API void ji_binding_destroy(JiBinding* binding) {
    if (!binding) return;
    ji_binding_detach(binding);
    ji_free(binding->source_path);
    ji_free(binding);
}

JI_API void ji_binding_apply(JiBinding* binding) {
    if (!binding) return;

    /* For path-based bindings, try to resolve source from data context */
    if (!binding->source && binding->source_path && binding->target) {
        void* ctx = ji_object_get_data_context(binding->target);
        if (ctx) {
            /* For now, treat data context as a JiObject */
            JiObject* ctx_obj = (JiObject*)ctx;
            JiPropertyId prop_id = ji_property_from_name(binding->source_path,
                                                          ji_object_type_id(ctx_obj));
            if (prop_id != JI_PROPERTY_INVALID) {
                binding->source = ctx_obj;
                binding->source_prop = prop_id;
            }
        }
    }

    /* Initial value transfer for OneWay, TwoWay, OneTime */
    if (binding->source && binding->target &&
        binding->source_prop != JI_PROPERTY_INVALID &&
        binding->target_prop != JI_PROPERTY_INVALID) {

        if (binding->mode == JI_BINDING_ONE_WAY ||
            binding->mode == JI_BINDING_TWO_WAY ||
            binding->mode == JI_BINDING_ONE_TIME) {
            JiPropertyValue val;
            if (ji_object_get_property(binding->source, binding->source_prop, &val) == 0) {
                ji_object_set_property(binding->target, binding->target_prop, val);
            }
        } else if (binding->mode == JI_BINDING_ONE_WAY_TO_SOURCE) {
            JiPropertyValue val;
            if (ji_object_get_property(binding->target, binding->target_prop, &val) == 0) {
                ji_object_set_property(binding->source, binding->source_prop, val);
            }
        }
    }

    /* Subscribe to change notifications */
    if (binding->mode == JI_BINDING_ONE_WAY || binding->mode == JI_BINDING_TWO_WAY ||
        binding->mode == JI_BINDING_ONE_TIME) {
        if (binding->source) {
            binding->source_sub_id = ji_object_add_property_changed(
                binding->source, binding_source_changed, binding);
        }
    }

    if (binding->mode == JI_BINDING_TWO_WAY || binding->mode == JI_BINDING_ONE_WAY_TO_SOURCE) {
        if (binding->target) {
            binding->target_sub_id = ji_object_add_property_changed(
                binding->target, binding_target_changed, binding);
        }
    }

    binding->is_active = true;
}

JI_API void ji_binding_detach(JiBinding* binding) {
    if (!binding || !binding->is_active) return;

    /* Unsubscribe from notifications */
    if (binding->source && binding->source_sub_id >= 0) {
        ji_object_remove_property_changed(binding->source, binding->source_sub_id);
        binding->source_sub_id = -1;
    }

    if (binding->target && binding->target_sub_id >= 0) {
        ji_object_remove_property_changed(binding->target, binding->target_sub_id);
        binding->target_sub_id = -1;
    }

    binding->is_active = false;
}

JI_API bool ji_binding_is_active(const JiBinding* binding) {
    return binding ? binding->is_active : false;
}

JI_API const char* ji_binding_mode_str(JiBindingMode mode) {
    switch (mode) {
        case JI_BINDING_ONE_WAY:           return "OneWay";
        case JI_BINDING_TWO_WAY:           return "TwoWay";
        case JI_BINDING_ONE_TIME:          return "OneTime";
        case JI_BINDING_ONE_WAY_TO_SOURCE: return "OneWayToSource";
        default:                           return "Unknown";
    }
}

JI_API JiBindingMode ji_binding_mode_from_str(const char* str) {
    if (!str) return JI_BINDING_ONE_WAY;
    if (strcmp(str, "TwoWay") == 0)           return JI_BINDING_TWO_WAY;
    if (strcmp(str, "OneTime") == 0)          return JI_BINDING_ONE_TIME;
    if (strcmp(str, "OneWayToSource") == 0)   return JI_BINDING_ONE_WAY_TO_SOURCE;
    return JI_BINDING_ONE_WAY;
}

/* =========================================================================
 * Binding Engine
 * ========================================================================= */

JI_API void ji_binding_engine_init(JiBindingEngine* engine) {
    if (!engine) return;
    memset(engine, 0, sizeof(JiBindingEngine));
    engine->bindings = NULL;
    engine->count = 0;
    engine->capacity = 0;
    engine->updating = false;
}

JI_API void ji_binding_engine_destroy(JiBindingEngine* engine) {
    if (!engine) return;
    for (int i = 0; i < engine->count; i++) {
        ji_binding_destroy(engine->bindings[i]);
    }
    ji_free(engine->bindings);
    engine->bindings = NULL;
    engine->count = 0;
    engine->capacity = 0;
}

JI_API JiBinding* ji_binding_engine_add(JiBindingEngine* engine,
                                           JiObject* target, JiPropertyId target_prop,
                                           JiObject* source, JiPropertyId source_prop,
                                           JiBindingMode mode) {
    if (!engine) return NULL;

    JiBinding* binding = ji_binding_new(target, target_prop, source, source_prop, mode);
    if (!binding) return NULL;

    /* Grow array if needed */
    if (engine->count >= engine->capacity) {
        int new_cap = engine->capacity == 0 ? 8 : engine->capacity * 2;
        engine->bindings = (JiBinding**)ji_realloc(engine->bindings,
            (size_t)new_cap * sizeof(JiBinding*));
        engine->capacity = new_cap;
    }

    engine->bindings[engine->count++] = binding;
    ji_binding_apply(binding);

    return binding;
}

JI_API JiBinding* ji_binding_engine_add_path(JiBindingEngine* engine,
                                                JiObject* target, JiPropertyId target_prop,
                                                const char* source_path,
                                                JiBindingMode mode) {
    if (!engine) return NULL;

    JiBinding* binding = ji_binding_new_with_path(target, target_prop, source_path, mode);
    if (!binding) return NULL;

    if (engine->count >= engine->capacity) {
        int new_cap = engine->capacity == 0 ? 8 : engine->capacity * 2;
        engine->bindings = (JiBinding**)ji_realloc(engine->bindings,
            (size_t)new_cap * sizeof(JiBinding*));
        engine->capacity = new_cap;
    }

    engine->bindings[engine->count++] = binding;
    ji_binding_apply(binding);

    return binding;
}

JI_API void ji_binding_engine_remove(JiBindingEngine* engine, JiBinding* binding) {
    if (!engine || !binding) return;

    for (int i = 0; i < engine->count; i++) {
        if (engine->bindings[i] == binding) {
            ji_binding_destroy(binding);
            /* Shift remaining */
            for (int j = i; j < engine->count - 1; j++) {
                engine->bindings[j] = engine->bindings[j + 1];
            }
            engine->count--;
            return;
        }
    }
}

JI_API int ji_binding_engine_count(const JiBindingEngine* engine) {
    return engine ? engine->count : 0;
}
