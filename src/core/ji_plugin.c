#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_plugin.c
 * @brief Plugin SDK implementation — manager, registration, lookup, sandbox.
 */

#include "jiui/ji_plugin.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Plugin Manager Lifecycle
 * ========================================================================= */

JiPluginManager* ji_plugin_manager_new(void)
{
    JiPluginManager* mgr = (JiPluginManager*)calloc(1, sizeof(JiPluginManager));
    if (!mgr) return NULL;
    mgr->plugin_capacity = 16;
    mgr->plugins = (JiPlugin**)calloc(mgr->plugin_capacity, sizeof(JiPlugin*));
    mgr->registry_capacity = 64;
    mgr->registry = (JiPluginEntry*)calloc(mgr->registry_capacity, sizeof(JiPluginEntry));
    return mgr;
}

void ji_plugin_manager_free(JiPluginManager* mgr)
{
    if (!mgr) return;
    ji_plugin_unload_all(mgr);
    free(mgr->plugins);
    free(mgr->registry);
    free(mgr);
}

/* =========================================================================
 * Internal Helpers
 * ========================================================================= */

static void ji_plugin_set_error(JiPluginManager* mgr, const char* msg)
{
    if (!mgr || !msg) return;
    strncpy(mgr->error_msg, msg, sizeof(mgr->error_msg) - 1);
    mgr->error_msg[sizeof(mgr->error_msg) - 1] = '\0';
}

static void ji_plugin_set_error_plug(JiPlugin* p, const char* msg)
{
    if (!p || !msg) return;
    strncpy(p->error_msg, msg, sizeof(p->error_msg) - 1);
    p->error_msg[sizeof(p->error_msg) - 1] = '\0';
}

bool ji_plugin_grow_plugins(JiPluginManager* mgr)
{
    if (mgr->plugin_count < mgr->plugin_capacity) return true;
    int new_cap = mgr->plugin_capacity * 2;
    JiPlugin** new_arr = (JiPlugin**)realloc(mgr->plugins, new_cap * sizeof(JiPlugin*));
    if (!new_arr) return false;
    mgr->plugins = new_arr;
    mgr->plugin_capacity = new_cap;
    return true;
}

bool ji_plugin_grow_registry(JiPluginManager* mgr)
{
    if (mgr->registry_count < mgr->registry_capacity) return true;
    int new_cap = mgr->registry_capacity * 2;
    JiPluginEntry* new_arr = (JiPluginEntry*)realloc(mgr->registry,
                                                      new_cap * sizeof(JiPluginEntry));
    if (!new_arr) return false;
    mgr->registry = new_arr;
    mgr->registry_capacity = new_cap;
    return true;
}

/* =========================================================================
 * Plugin Registration
 * ========================================================================= */

static bool ji_plugin_add_entry(JiPlugin* plugin, const char* name,
                                  JiPluginType type,
                                  JiPluginCreateFn create,
                                  JiPluginDestroyFn destroy)
{
    if (!plugin || !name || !create) return false;
    if (plugin->entry_count >= JI_PLUGIN_MAX_ENTRIES) {
        ji_plugin_set_error_plug(plugin, "max entries reached");
        return false;
    }
    JiPluginEntry* e = &plugin->entries[plugin->entry_count++];
    strncpy(e->name, name, JI_PLUGIN_NAME_MAX - 1);
    e->name[JI_PLUGIN_NAME_MAX - 1] = '\0';
    e->type = type;
    e->create = create;
    e->destroy = destroy;
    e->instance = NULL;
    e->active = false;
    return true;
}

bool ji_plugin_register_widget(JiPlugin* plugin, const char* name,
                                 JiPluginCreateFn create,
                                 JiPluginDestroyFn destroy)
{
    return ji_plugin_add_entry(plugin, name, JI_PLUGIN_WIDGET, create, destroy);
}

bool ji_plugin_register_effect(JiPlugin* plugin, const char* name,
                                 JiPluginCreateFn create)
{
    return ji_plugin_add_entry(plugin, name, JI_PLUGIN_EFFECT, create, NULL);
}

bool ji_plugin_register_theme(JiPlugin* plugin, const char* name,
                                JiPluginCreateFn create)
{
    return ji_plugin_add_entry(plugin, name, JI_PLUGIN_THEME, create, NULL);
}

bool ji_plugin_register_tool(JiPlugin* plugin, const char* name,
                               JiPluginCreateFn create)
{
    return ji_plugin_add_entry(plugin, name, JI_PLUGIN_TOOL, create, NULL);
}

/* =========================================================================
 * Plugin Lookup
 * ========================================================================= */

JiPluginEntry* ji_plugin_find_entry(JiPluginManager* mgr, const char* name)
{
    if (!mgr || !name) return NULL;
    /* Search global registry first */
    for (int i = 0; i < mgr->registry_count; i++) {
        if (strcmp(mgr->registry[i].name, name) == 0)
            return &mgr->registry[i];
    }
    /* Search individual plugins */
    for (int i = 0; i < mgr->plugin_count; i++) {
        JiPlugin* p = mgr->plugins[i];
        if (!p) continue;
        for (int j = 0; j < p->entry_count; j++) {
            if (strcmp(p->entries[j].name, name) == 0)
                return &p->entries[j];
        }
    }
    return NULL;
}

JiPlugin* ji_plugin_find(JiPluginManager* mgr, const char* name)
{
    if (!mgr || !name) return NULL;
    for (int i = 0; i < mgr->plugin_count; i++) {
        if (mgr->plugins[i] && strcmp(mgr->plugins[i]->manifest.name, name) == 0)
            return mgr->plugins[i];
    }
    return NULL;
}

JiPlugin** ji_plugin_get_all(JiPluginManager* mgr, int* count)
{
    if (!mgr || !count) return NULL;
    *count = mgr->plugin_count;
    return mgr->plugins;
}

/* =========================================================================
 * Plugin Activation
 * ========================================================================= */

bool ji_plugin_activate(JiPlugin* plugin)
{
    if (!plugin) return false;
    if (plugin->state == JI_PLUGIN_ACTIVE) return true;
    if (plugin->state != JI_PLUGIN_LOADED) return false;

    /* Call init function if available */
    if (plugin->init_fn) {
        plugin->init_fn(NULL);
    }

    /* Activate all entries */
    for (int i = 0; i < plugin->entry_count; i++) {
        JiPluginEntry* e = &plugin->entries[i];
        if (!e->active && e->create) {
            e->instance = e->create();
            e->active = (e->instance != NULL);
        }
    }

    plugin->state = JI_PLUGIN_ACTIVE;
    return true;
}

bool ji_plugin_deactivate(JiPlugin* plugin)
{
    if (!plugin) return false;
    if (plugin->state != JI_PLUGIN_ACTIVE) return true;

    /* Deactivate all entries */
    for (int i = 0; i < plugin->entry_count; i++) {
        JiPluginEntry* e = &plugin->entries[i];
        if (e->active && e->destroy && e->instance) {
            e->destroy(e->instance);
            e->instance = NULL;
            e->active = false;
        }
    }

    /* Call shutdown function if available */
    if (plugin->shutdown_fn) {
        plugin->shutdown_fn();
    }

    plugin->state = JI_PLUGIN_LOADED;
    return true;
}

bool ji_plugin_is_active(JiPlugin* plugin)
{
    if (!plugin) return false;
    return plugin->state == JI_PLUGIN_ACTIVE;
}

/* =========================================================================
 * Plugin Sandbox
 * ========================================================================= */

static bool g_sandbox_enabled = false;

void ji_plugin_set_sandbox(JiPluginManager* mgr, bool enabled)
{
    (void)mgr;
    g_sandbox_enabled = enabled;
}

bool ji_plugin_is_sandboxed(JiPluginManager* mgr)
{
    (void)mgr;
    return g_sandbox_enabled;
}

/* =========================================================================
 * Error Retrieval
 * ========================================================================= */

const char* ji_plugin_get_error(JiPluginManager* mgr)
{
    if (!mgr) return NULL;
    return mgr->error_msg;
}
