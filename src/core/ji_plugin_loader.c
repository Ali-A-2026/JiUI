#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_plugin_loader.c
 * @brief Dynamic plugin loading (dlopen/dlclose) and manifest parsing.
 */

#include "jiui/ji_plugin.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declarations from ji_plugin.c */
bool ji_plugin_grow_plugins(JiPluginManager* mgr);
bool ji_plugin_grow_registry(JiPluginManager* mgr);

/* Dynamic loading headers */
#ifdef _WIN32
  #include <windows.h>
  typedef HMODULE dl_handle_t;
  static dl_handle_t ji_dl_open(const char* path) { return LoadLibraryA(path); }
  static void* ji_dl_sym(dl_handle_t h, const char* name) { return (void*)GetProcAddress(h, name); }
  static void ji_dl_close(dl_handle_t h) { FreeLibrary(h); }
  #define JI_DL_EXT ".dll"
#else
  #include <dlfcn.h>
  typedef void* dl_handle_t;
  static dl_handle_t ji_dl_open(const char* path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
  static void* ji_dl_sym(dl_handle_t h, const char* name) { return dlsym(h, name); }
  static void ji_dl_close(dl_handle_t h) { dlclose(h); }
  #define JI_DL_EXT ".so"
#endif

/* =========================================================================
 * Internal: set error on manager
 * ========================================================================= */

static void set_mgr_error(JiPluginManager* mgr, const char* msg)
{
    if (!mgr || !msg) return;
    strncpy(mgr->error_msg, msg, sizeof(mgr->error_msg) - 1);
    mgr->error_msg[sizeof(mgr->error_msg) - 1] = '\0';
}

/* =========================================================================
 * Plugin Loading / Unloading
 * ========================================================================= */

JiPlugin* ji_plugin_load(JiPluginManager* mgr, const char* path)
{
    if (!mgr || !path) {
        set_mgr_error(mgr, "null manager or path");
        return NULL;
    }

    dl_handle_t handle = ji_dl_open(path);
    if (!handle) {
#ifdef _WIN32
        set_mgr_error(mgr, "LoadLibrary failed");
#else
        const char* err = dlerror();
        set_mgr_error(mgr, err ? err : "dlopen failed");
#endif
        return NULL;
    }

    JiPlugin* plugin = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    if (!plugin) {
        ji_dl_close(handle);
        set_mgr_error(mgr, "out of memory");
        return NULL;
    }

    plugin->handle = (void*)handle;
    plugin->state = JI_PLUGIN_LOADED;
    strncpy(plugin->path, path, sizeof(plugin->path) - 1);

    /* Look for exported functions */
    plugin->init_fn = (JiPluginInitFn)ji_dl_sym(handle, "ji_plugin_init");
    plugin->shutdown_fn = (JiPluginShutdownFn)ji_dl_sym(handle, "ji_plugin_shutdown");

    /* Try to load manifest function */
    typedef const char* (*manifest_fn_t)(void);
    manifest_fn_t manifest_fn = (manifest_fn_t)ji_dl_sym(handle, "ji_plugin_manifest");
    if (manifest_fn) {
        const char* json = manifest_fn();
        if (json) {
            ji_plugin_manifest_parse(json, &plugin->manifest);
        }
    }

    /* If manifest name is empty, use filename */
    if (plugin->manifest.name[0] == '\0') {
        const char* slash = strrchr(path, '/');
        const char* base = slash ? slash + 1 : path;
        strncpy(plugin->manifest.name, base, JI_PLUGIN_NAME_MAX - 1);
        /* Strip extension */
        char* dot = strrchr(plugin->manifest.name, '.');
        if (dot) *dot = '\0';
    }

    /* Call init if available */
    if (plugin->init_fn) {
        plugin->init_fn(NULL);
    }

    /* Add to manager */
    if (!ji_plugin_grow_plugins(mgr)) {
        ji_dl_close(handle);
        free(plugin);
        set_mgr_error(mgr, "plugin list full");
        return NULL;
    }
    mgr->plugins[mgr->plugin_count++] = plugin;

    return plugin;
}

void ji_plugin_unload(JiPluginManager* mgr, JiPlugin* plugin)
{
    if (!mgr || !plugin) return;

    /* Deactivate first */
    if (plugin->state == JI_PLUGIN_ACTIVE) {
        ji_plugin_deactivate(plugin);
    }

    /* Call shutdown if available */
    if (plugin->shutdown_fn) {
        plugin->shutdown_fn();
    }

    /* Close dynamic library */
    if (plugin->handle) {
        ji_dl_close((dl_handle_t)plugin->handle);
        plugin->handle = NULL;
    }

    /* Remove from manager's list */
    for (int i = 0; i < mgr->plugin_count; i++) {
        if (mgr->plugins[i] == plugin) {
            /* Shift remaining plugins down */
            for (int j = i; j < mgr->plugin_count - 1; j++)
                mgr->plugins[j] = mgr->plugins[j + 1];
            mgr->plugin_count--;
            break;
        }
    }

    /* Free manifest dependencies */
    if (plugin->manifest.dependencies) {
        for (int i = 0; i < plugin->manifest.dep_count; i++)
            free(plugin->manifest.dependencies[i]);
        free(plugin->manifest.dependencies);
    }

    free(plugin);
}

void ji_plugin_unload_all(JiPluginManager* mgr)
{
    if (!mgr) return;
    while (mgr->plugin_count > 0) {
        ji_plugin_unload(mgr, mgr->plugins[0]);
    }
}

/* =========================================================================
 * Manifest Parsing (simplified JSON)
 * ========================================================================= */

/* Skip whitespace */
static const char* skip_ws(const char* s)
{
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}

/* Extract string value after key — very simplified JSON parser */
static bool extract_string(const char* json, const char* key, char* out, int out_size)
{
    if (!json || !key || !out || out_size <= 0) return false;
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* pos = strstr(json, pattern);
    if (!pos) return false;
    pos += strlen(pattern);
    pos = skip_ws(pos);
    if (*pos != ':') return false;
    pos++;
    pos = skip_ws(pos);
    if (*pos != '"') return false;
    pos++;
    int i = 0;
    while (*pos && *pos != '"' && i < out_size - 1) {
        if (*pos == '\\' && pos[1]) pos++;
        out[i++] = *pos++;
    }
    out[i] = '\0';
    return true;
}

static bool extract_int(const char* json, const char* key, int* out)
{
    if (!json || !key || !out) return false;
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* pos = strstr(json, pattern);
    if (!pos) return false;
    pos += strlen(pattern);
    pos = skip_ws(pos);
    if (*pos != ':') return false;
    pos++;
    pos = skip_ws(pos);
    *out = atoi(pos);
    return true;
}

bool ji_plugin_manifest_parse(const char* json, JiPluginManifest* manifest)
{
    if (!json || !manifest) return false;
    memset(manifest, 0, sizeof(JiPluginManifest));

    extract_string(json, "name", manifest->name, JI_PLUGIN_NAME_MAX);
    extract_string(json, "version", manifest->version, JI_PLUGIN_VERSION_MAX);
    extract_string(json, "description", manifest->description, JI_PLUGIN_DESC_MAX);
    extract_string(json, "author", manifest->author, JI_PLUGIN_AUTHOR_MAX);

    int type = 0;
    if (extract_int(json, "type", &type))
        manifest->type = (JiPluginType)type;

    int min_api = 0, max_api = 0;
    if (extract_int(json, "min_api", &min_api))
        manifest->min_api_version = (uint32_t)min_api;
    if (extract_int(json, "max_api", &max_api))
        manifest->max_api_version = (uint32_t)max_api;

    return manifest->name[0] != '\0';
}

int ji_plugin_manifest_to_json(const JiPluginManifest* manifest,
                                 char* buf, int buf_size)
{
    if (!manifest || !buf || buf_size <= 0) return 0;
    return snprintf(buf, (size_t)buf_size,
        "{\n"
        "  \"name\": \"%s\",\n"
        "  \"version\": \"%s\",\n"
        "  \"description\": \"%s\",\n"
        "  \"author\": \"%s\",\n"
        "  \"type\": %d,\n"
        "  \"min_api\": %u,\n"
        "  \"max_api\": %u\n"
        "}\n",
        manifest->name,
        manifest->version,
        manifest->description,
        manifest->author,
        (int)manifest->type,
        manifest->min_api_version,
        manifest->max_api_version);
}
