/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_loader.h
 * @brief Runtime loader — high-level API for loading .ji XML files into
 *        live object trees with resource resolution and binding application.
 *
 * This is the primary entry point for applications using the XML-based
 * .ji declarative markup. It orchestrates:
 *   1. XML parsing (lexer + parser)
 *   2. AST → JiObject tree building
 *   3. Resource dictionary population
 *   4. Binding creation and application
 *   5. Named element registration
 */

#ifndef JIUI_LOADER_H
#define JIUI_LOADER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_object.h"
#include "ji_binding.h"
#include "ji_event.h"
#include "ji_style.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Loader result — contains the loaded tree and associated metadata
 * ========================================================================= */
typedef struct JiLoadResult {
    JiObject*           root;           /* Root object of the loaded tree */
    JiBindingEngine     bindings;       /* Binding engine with all active bindings */
    JiResourceDictionary* resources;    /* Merged resource dictionary */
    JiStyle**           styles;        /* Array of parsed styles */
    int                 style_count;   /* Number of parsed styles */
    int                 style_capacity;/* Allocated capacity */
    JiEventBus          events;        /* Event bus with all subscriptions */
    bool                has_error;      /* True if loading failed */
    char                error_msg[512]; /* Error description */
} JiLoadResult;

/* =========================================================================
 * Loader API
 * ========================================================================= */

/**
 * Load a .ji XML file and build a live object tree.
 *
 * @param filepath  Path to the .ji XML file.
 * @return         Load result containing the root object, bindings, and resources.
 *                 Caller must call ji_load_result_destroy() when done.
 */
JI_API JiLoadResult ji_load_file(const char* filepath);

/**
 * Load a .ji XML string and build a live object tree.
 *
 * @param source  XML source text.
 * @return        Load result.
 */
JI_API JiLoadResult ji_load_string(const char* source);

/**
 * Destroy a load result and free all associated resources.
 * This destroys the binding engine and resource dictionary,
 * but does NOT destroy the root object (caller must release it).
 */
JI_API void ji_load_result_destroy(JiLoadResult* result);

/**
 * Look up a named element in the loaded tree by x:Name.
 *
 * @param root  Root of the object tree.
 * @param name  Element name to find.
 * @return      The named object, or NULL if not found.
 */
JI_API JiObject* ji_load_find_name(JiObject* root, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_LOADER_H */
