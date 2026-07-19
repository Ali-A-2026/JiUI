/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_plugin.h
 * @brief Plugin SDK — dynamic plugin loading, widget/effect/theme registration,
 *        plugin manifest parsing, and plugin sandboxing.
 */

#ifndef JIUI_PLUGIN_H
#define JIUI_PLUGIN_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Plugin Constants
 * ========================================================================= */

#define JI_PLUGIN_NAME_MAX     128
#define JI_PLUGIN_VERSION_MAX  32
#define JI_PLUGIN_DESC_MAX     256
#define JI_PLUGIN_AUTHOR_MAX   128
#define JI_PLUGIN_MAX_ENTRIES  64

/* =========================================================================
 * Plugin Types
 * ========================================================================= */

typedef enum JiPluginType {
    JI_PLUGIN_WIDGET   = 0,   /* Custom widget plugin */
    JI_PLUGIN_EFFECT   = 1,   /* Visual effect plugin */
    JI_PLUGIN_THEME    = 2,   /* Theme/skin plugin */
    JI_PLUGIN_TOOL     = 3,   /* Tool plugin (profiler, debugger, etc.) */
    JI_PLUGIN_LAYOUT   = 4,   /* Custom layout plugin */
    JI_PLUGIN_RENDERER = 5    /* Custom renderer plugin */
} JiPluginType;

typedef enum JiPluginState {
    JI_PLUGIN_UNLOADED = 0,
    JI_PLUGIN_LOADED  = 1,
    JI_PLUGIN_ACTIVE  = 2,
    JI_PLUGIN_ERROR   = 3
} JiPluginState;

/** Plugin manifest (parsed from JSON or plugin metadata). */
typedef struct JiPluginManifest {
    char name[JI_PLUGIN_NAME_MAX];
    char version[JI_PLUGIN_VERSION_MAX];
    char description[JI_PLUGIN_DESC_MAX];
    char author[JI_PLUGIN_AUTHOR_MAX];
    JiPluginType type;
    char** dependencies;     /* Other plugin names this depends on */
    int   dep_count;
    uint32_t min_api_version;
    uint32_t max_api_version;
} JiPluginManifest;

/** Function pointers the plugin must export. */
typedef void* (*JiPluginCreateFn)(void);
typedef void  (*JiPluginDestroyFn)(void* instance);
typedef void  (*JiPluginInitFn)(void* api);
typedef void  (*JiPluginShutdownFn)(void);

/** A registered plugin entry (widget/effect/theme/etc). */
typedef struct JiPluginEntry {
    char name[JI_PLUGIN_NAME_MAX];
    JiPluginType type;
    JiPluginCreateFn  create;
    JiPluginDestroyFn destroy;
    void* instance;
    bool active;
} JiPluginEntry;

/** Main plugin handle. */
typedef struct JiPlugin {
    JiPluginManifest manifest;
    JiPluginState    state;
    void*            handle;       /* dlopen handle */
    char             path[512];    /* File path */
    JiPluginInitFn     init_fn;
    JiPluginShutdownFn shutdown_fn;
    JiPluginEntry     entries[JI_PLUGIN_MAX_ENTRIES];
    int               entry_count;
    char              error_msg[256];
} JiPlugin;

/** Plugin manager — tracks all loaded plugins. */
typedef struct JiPluginManager {
    JiPlugin** plugins;
    int        plugin_count;
    int        plugin_capacity;
    JiPluginEntry* registry;  /* Global registry of all entries */
    int       registry_count;
    int       registry_capacity;
    char      error_msg[256];
} JiPluginManager;

/* =========================================================================
 * Plugin Manager Lifecycle
 * ========================================================================= */

JI_API JiPluginManager* ji_plugin_manager_new(void);
JI_API void             ji_plugin_manager_free(JiPluginManager* mgr);

/* =========================================================================
 * Plugin Loading / Unloading
 * ========================================================================= */

/** Load a plugin from a .so/.dll file. Returns NULL on failure. */
JI_API JiPlugin* ji_plugin_load(JiPluginManager* mgr, const char* path);

/** Unload a specific plugin. */
JI_API void      ji_plugin_unload(JiPluginManager* mgr, JiPlugin* plugin);

/** Unload all plugins. */
JI_API void      ji_plugin_unload_all(JiPluginManager* mgr);

/** Get last error message. */
JI_API const char* ji_plugin_get_error(JiPluginManager* mgr);

/* =========================================================================
 * Plugin Registration
 * ========================================================================= */

JI_API bool ji_plugin_register_widget(JiPlugin* plugin, const char* name,
                                       JiPluginCreateFn create,
                                       JiPluginDestroyFn destroy);

JI_API bool ji_plugin_register_effect(JiPlugin* plugin, const char* name,
                                       JiPluginCreateFn create);

JI_API bool ji_plugin_register_theme(JiPlugin* plugin, const char* name,
                                      JiPluginCreateFn create);

JI_API bool ji_plugin_register_tool(JiPlugin* plugin, const char* name,
                                     JiPluginCreateFn create);

/* =========================================================================
 * Plugin Lookup
 * ========================================================================= */

/** Find a plugin entry by name across all loaded plugins. */
JI_API JiPluginEntry* ji_plugin_find_entry(JiPluginManager* mgr, const char* name);

/** Find a plugin by name. */
JI_API JiPlugin* ji_plugin_find(JiPluginManager* mgr, const char* name);

/** Get all loaded plugins. */
JI_API JiPlugin** ji_plugin_get_all(JiPluginManager* mgr, int* count);

/* =========================================================================
 * Plugin Manifest Parsing (simplified JSON)
 * ========================================================================= */

/** Parse a plugin manifest from a JSON string. */
JI_API bool ji_plugin_manifest_parse(const char* json, JiPluginManifest* manifest);

/** Write a plugin manifest to a JSON string. */
JI_API int  ji_plugin_manifest_to_json(const JiPluginManifest* manifest,
                                        char* buf, int buf_size);

/* =========================================================================
 * Plugin Sandbox
 * ========================================================================= */

/** Enable sandbox mode — restricts plugin memory and API access. */
JI_API void ji_plugin_set_sandbox(JiPluginManager* mgr, bool enabled);

/** Check if sandbox mode is enabled. */
JI_API bool ji_plugin_is_sandboxed(JiPluginManager* mgr);

/* =========================================================================
 * Plugin Activation
 * ========================================================================= */

JI_API bool ji_plugin_activate(JiPlugin* plugin);
JI_API bool ji_plugin_deactivate(JiPlugin* plugin);
JI_API bool ji_plugin_is_active(JiPlugin* plugin);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PLUGIN_H */
