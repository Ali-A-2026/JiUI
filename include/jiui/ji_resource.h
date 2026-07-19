/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_resource.h
 * @brief Resource dictionary — shared key-value resources for .ji files.
 *
 * This header provides the resource dictionary API. The implementation
 * lives in the style module (ji_style.h / ji_style.c).
 */

#ifndef JIUI_RESOURCE_H
#define JIUI_RESOURCE_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_property.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Resource dictionary entry
 * ========================================================================= */
typedef struct JiResourceEntry {
    char*           key;
    JiPropertyValue value;
} JiResourceEntry;

/* =========================================================================
 * Resource dictionary — maps string keys to typed values
 * ========================================================================= */
typedef struct JiResourceDictionary {
    JiResourceEntry* entries;
    int              entry_count;
    int              entry_capacity;
} JiResourceDictionary;

/* =========================================================================
 * Resource Dictionary API
 * ========================================================================= */

/** Create a new resource dictionary. */
JI_API JiResourceDictionary* ji_resource_dict_new(void);

/** Destroy a resource dictionary. */
JI_API void ji_resource_dict_destroy(JiResourceDictionary* dict);

/** Add or update a string resource. */
JI_API void ji_resource_dict_set_string(JiResourceDictionary* dict,
                                          const char* key, const char* value);

/** Add or update a double resource. */
JI_API void ji_resource_dict_set_double(JiResourceDictionary* dict,
                                          const char* key, double value);

/** Add or update a color resource. */
JI_API void ji_resource_dict_set_color(JiResourceDictionary* dict,
                                         const char* key, uint32_t argb);

/** Add or update an integer resource. */
JI_API void ji_resource_dict_set_int(JiResourceDictionary* dict,
                                       const char* key, int value);

/** Add or update a boolean resource. */
JI_API void ji_resource_dict_set_bool(JiResourceDictionary* dict,
                                        const char* key, bool value);

/** Look up a resource by key. Returns true if found. */
JI_API bool ji_resource_dict_try_get(const JiResourceDictionary* dict,
                                       const char* key, JiPropertyValue* out_val);

/** Look up a string resource by key. Returns NULL if not found. */
JI_API const char* ji_resource_dict_get_string(const JiResourceDictionary* dict,
                                                 const char* key);

/** Check if a key exists in the dictionary. */
JI_API bool ji_resource_dict_has(const JiResourceDictionary* dict, const char* key);

/** Remove a resource by key. Returns true if found and removed. */
JI_API bool ji_resource_dict_remove(JiResourceDictionary* dict, const char* key);

/** Get the number of resources. */
JI_API int ji_resource_dict_get_count(const JiResourceDictionary* dict);

/** Merge another dictionary into this one (copies entries). */
JI_API void ji_resource_dict_merge(JiResourceDictionary* dict,
                                      const JiResourceDictionary* other);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_RESOURCE_H */
