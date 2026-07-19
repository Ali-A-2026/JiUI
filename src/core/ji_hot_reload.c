/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_hot_reload.c
 * @brief Hot reload engine implementation for JiML files.
 *
 * Watches .jiml files for changes via the file watcher, re-parses and
 * re-compiles them on change, and notifies the application via a callback.
 * State preservation is supported through a simple property snapshot.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_hot_reload.h"
#include "jiui/ji_jiml.h"
#include "jiui/ji_object.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_property.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* =========================================================================
 * Constants
 * ========================================================================= */

#define JI_HOT_RELOAD_MAX_WATCHED 32
#define JI_HOT_RELOAD_MAX_STATE_PROPS 64
#define JI_HOT_RELOAD_MAX_PROP_NAME 128
#define JI_HOT_RELOAD_MAX_PROP_VALUE 512

/* =========================================================================
 * State Preservation
 * ========================================================================= */

typedef struct JiSavedProp {
    char name[JI_HOT_RELOAD_MAX_PROP_NAME];
    char value[JI_HOT_RELOAD_MAX_PROP_VALUE];
} JiSavedProp;

typedef struct JiSavedState {
    JiSavedProp props[JI_HOT_RELOAD_MAX_STATE_PROPS];
    int prop_count;
} JiSavedState;

/* =========================================================================
 * Watched File Entry
 * ========================================================================= */

typedef struct JiWatchedEntry {
    char            filepath[512];
    void*           context;       /* Data context for binding resolution */
    JiObject*       root;          /* Current root object (owned by engine) */
    JiSavedState*   saved_state;   /* Saved state for preservation (may be NULL) */
    JiHotReloadResult last_error;  /* Last error result */
    bool            active;        /* Whether this slot is in use */
} JiWatchedEntry;

/* =========================================================================
 * Hot Reload Engine State
 * ========================================================================= */

struct JiHotReloadEngine {
    JiFileWatcher*       watcher;
    JiWatchedEntry       entries[JI_HOT_RELOAD_MAX_WATCHED];
    int                  count;
    JiHotReloadCallback  callback;
    void*                callback_user_data;
    bool                 enabled;
};

/* =========================================================================
 * Helper: find a watched entry by filepath
 * ========================================================================= */

static JiWatchedEntry* find_entry(JiHotReloadEngine* engine, const char* filepath) {
    if (!engine || !filepath) return NULL;
    for (int i = 0; i < JI_HOT_RELOAD_MAX_WATCHED; i++) {
        if (engine->entries[i].active &&
            strcmp(engine->entries[i].filepath, filepath) == 0) {
            return &engine->entries[i];
        }
    }
    return NULL;
}

static const JiWatchedEntry* find_entry_const(const JiHotReloadEngine* engine,
                                                const char* filepath) {
    if (!engine || !filepath) return NULL;
    for (int i = 0; i < JI_HOT_RELOAD_MAX_WATCHED; i++) {
        if (engine->entries[i].active &&
            strcmp(engine->entries[i].filepath, filepath) == 0) {
            return &engine->entries[i];
        }
    }
    return NULL;
}

/* =========================================================================
 * Helper: find a free entry slot
 * ========================================================================= */

static JiWatchedEntry* find_free_slot(JiHotReloadEngine* engine) {
    for (int i = 0; i < JI_HOT_RELOAD_MAX_WATCHED; i++) {
        if (!engine->entries[i].active) {
            return &engine->entries[i];
        }
    }
    return NULL;
}

/* =========================================================================
 * Helper: set result fields
 * ========================================================================= */

static JiHotReloadResult make_result(JiHotReloadErrorLevel level,
                                        const char* message,
                                        const char* filepath,
                                        int line,
                                        bool reloaded) {
    JiHotReloadResult r;
    memset(&r, 0, sizeof(r));
    r.level = level;
    r.line = line;
    r.reloaded = reloaded;
    if (message) {
        strncpy(r.message, message, sizeof(r.message) - 1);
    }
    if (filepath) {
        strncpy(r.filepath, filepath, sizeof(r.filepath) - 1);
    }
    return r;
}

/* =========================================================================
 * Helper: collect properties from an object tree (recursive)
 * ========================================================================= */

static void collect_props_recursive(JiObject* obj, JiSavedState* state) {
    if (!obj || !state || state->prop_count >= JI_HOT_RELOAD_MAX_STATE_PROPS) return;

    /* Try to read properties from this object */
    /* We use ji_object_property_count and ji_object_property_name/value if available */
    /* For now, we use a generic approach via the property API */
    /* Since the exact property enumeration API may vary, we do a best-effort scan */

    /* TODO: When property enumeration is available, iterate and save */

    /* Recurse into children (direct struct access) */
    for (int i = 0; i < obj->child_count; i++) {
        collect_props_recursive(obj->children[i], state);
    }
}

/* =========================================================================
 * Helper: restore properties to a new object tree (recursive)
 * ========================================================================= */

static void restore_props_recursive(JiObject* obj, const JiSavedState* state) {
    if (!obj || !state) return;

    /* TODO: When property enumeration is available, restore saved values */

    /* Recurse into children (direct struct access) */
    for (int i = 0; i < obj->child_count; i++) {
        restore_props_recursive(obj->children[i], state);
    }
}

/* =========================================================================
 * Helper: reload a single file
 * ========================================================================= */

static JiHotReloadResult reload_file(JiHotReloadEngine* engine,
                                        JiWatchedEntry* entry) {
    if (!engine || !entry) {
        return make_result(JI_HOT_RELOAD_ERROR, "Invalid arguments", NULL, 0, false);
    }

    /* Save state from old root */
    if (entry->root) {
        if (!entry->saved_state) {
            entry->saved_state = ji_calloc(1, sizeof(JiSavedState));
        }
        if (entry->saved_state) {
            entry->saved_state->prop_count = 0;
            collect_props_recursive(entry->root, entry->saved_state);
        }
    }

    /* Compile the file */
    JiTreeBuildResult build = ji_jiml_compile_file(entry->filepath, entry->context);

    if (!build.root) {
        /* Compilation failed */
        JiHotReloadResult result = make_result(
            JI_HOT_RELOAD_PARSE_ERROR,
            build.error_msg ? build.error_msg : "Compilation failed",
            entry->filepath,
            build.error_line,
            false
        );
        entry->last_error = result;
        return result;
    }

    /* Release old root */
    if (entry->root) {
        ji_ref_object_release(entry->root);
    }

    /* Set new root */
    entry->root = build.root;

    /* Restore state to new root */
    if (entry->saved_state) {
        restore_props_recursive(entry->root, entry->saved_state);
    }

    JiHotReloadResult result = make_result(JI_HOT_RELOAD_OK, "Reload successful",
                                             entry->filepath, 0, true);
    entry->last_error = result;
    return result;
}

/* =========================================================================
 * File watcher callback (used internally by poll)
 * ========================================================================= */

typedef struct JiReloadCallbackContext {
    JiHotReloadEngine* engine;
    int                reload_count;
} JiReloadCallbackContext;

static void file_changed_callback(const JiFileNotification* notif, void* user_data) {
    JiReloadCallbackContext* ctx = (JiReloadCallbackContext*)user_data;
    if (!notif || !ctx || !ctx->engine) return;

    /* Only handle modified and created events */
    if (notif->event != JI_FILE_EVENT_MODIFIED &&
        notif->event != JI_FILE_EVENT_CREATED) {
        return;
    }

    /* Find the entry for this file */
    JiWatchedEntry* entry = find_entry(ctx->engine, notif->filepath);
    if (!entry) return;

    /* Reload the file */
    JiHotReloadResult result = reload_file(ctx->engine, entry);

    /* Invoke user callback */
    if (ctx->engine->callback) {
        ctx->engine->callback(&result, entry->root, ctx->engine->callback_user_data);
    }

    if (result.reloaded) {
        ctx->reload_count++;
    }
}

/* =========================================================================
 * Hot Reload Engine API
 * ========================================================================= */

JI_API JiHotReloadEngine* ji_hot_reload_new(void) {
    JiHotReloadEngine* engine = ji_calloc(1, sizeof(JiHotReloadEngine));
    if (!engine) return NULL;

    engine->watcher = ji_file_watcher_new();
    if (!engine->watcher) {
        ji_free(engine);
        return NULL;
    }

    engine->enabled = true;
    engine->count = 0;
    engine->callback = NULL;
    engine->callback_user_data = NULL;

    return engine;
}

JI_API void ji_hot_reload_destroy(JiHotReloadEngine* engine) {
    if (!engine) return;

    /* Release all root objects and saved states */
    for (int i = 0; i < JI_HOT_RELOAD_MAX_WATCHED; i++) {
        JiWatchedEntry* entry = &engine->entries[i];
        if (entry->active) {
            if (entry->root) {
                ji_ref_object_release(entry->root);
            }
            if (entry->saved_state) {
                ji_free(entry->saved_state);
            }
        }
    }

    if (engine->watcher) {
        ji_file_watcher_destroy(engine->watcher);
    }

    ji_free(engine);
}

JI_API bool ji_hot_reload_watch(JiHotReloadEngine* engine,
                                   const char* filepath,
                                   void* context) {
    if (!engine || !filepath) return false;

    /* Check if already watching */
    if (find_entry(engine, filepath)) {
        return true;
    }

    /* Find a free slot */
    JiWatchedEntry* entry = find_free_slot(engine);
    if (!entry) return false;

    /* Initialize the entry */
    memset(entry, 0, sizeof(*entry));
    strncpy(entry->filepath, filepath, sizeof(entry->filepath) - 1);
    entry->context = context;
    entry->root = NULL;
    entry->saved_state = NULL;
    entry->active = true;
    memset(&entry->last_error, 0, sizeof(entry->last_error));

    /* Add to file watcher */
    if (!ji_file_watcher_add(engine->watcher, filepath)) {
        entry->active = false;
        return false;
    }

    /* Attempt initial load */
    JiHotReloadResult result = reload_file(engine, entry);
    if (result.level == JI_HOT_RELOAD_ERROR ||
        result.level == JI_HOT_RELOAD_PARSE_ERROR) {
        /* Initial load failed, but we keep watching */
        /* The error is stored in entry->last_error */
    }

    engine->count++;
    return true;
}

JI_API bool ji_hot_reload_unwatch(JiHotReloadEngine* engine,
                                     const char* filepath) {
    if (!engine || !filepath) return false;

    JiWatchedEntry* entry = find_entry(engine, filepath);
    if (!entry) return false;

    /* Release resources */
    if (entry->root) {
        ji_ref_object_release(entry->root);
        entry->root = NULL;
    }
    if (entry->saved_state) {
        ji_free(entry->saved_state);
        entry->saved_state = NULL;
    }

    /* Remove from file watcher */
    ji_file_watcher_remove(engine->watcher, filepath);

    /* Deactivate the entry */
    entry->active = false;
    memset(entry->filepath, 0, sizeof(entry->filepath));
    engine->count--;

    return true;
}

JI_API void ji_hot_reload_set_callback(JiHotReloadEngine* engine,
                                          JiHotReloadCallback callback,
                                          void* user_data) {
    if (!engine) return;
    engine->callback = callback;
    engine->callback_user_data = user_data;
}

JI_API void ji_hot_reload_set_enabled(JiHotReloadEngine* engine, bool enabled) {
    if (!engine) return;
    engine->enabled = enabled;
}

JI_API bool ji_hot_reload_is_enabled(const JiHotReloadEngine* engine) {
    return engine ? engine->enabled : false;
}

JI_API int ji_hot_reload_poll(JiHotReloadEngine* engine) {
    if (!engine || !engine->enabled) return 0;

    JiReloadCallbackContext ctx;
    ctx.engine = engine;
    ctx.reload_count = 0;

    ji_file_watcher_poll(engine->watcher, file_changed_callback, &ctx);

    return ctx.reload_count;
}

JI_API int ji_hot_reload_reload_all(JiHotReloadEngine* engine) {
    if (!engine) return 0;

    int count = 0;
    for (int i = 0; i < JI_HOT_RELOAD_MAX_WATCHED; i++) {
        JiWatchedEntry* entry = &engine->entries[i];
        if (!entry->active) continue;

        JiHotReloadResult result = reload_file(engine, entry);

        if (engine->callback) {
            engine->callback(&result, entry->root, engine->callback_user_data);
        }

        if (result.reloaded) {
            count++;
        }
    }

    return count;
}

JI_API JiHotReloadResult ji_hot_reload_reload(JiHotReloadEngine* engine,
                                                  const char* filepath) {
    if (!engine || !filepath) {
        return make_result(JI_HOT_RELOAD_ERROR, "Invalid arguments", NULL, 0, false);
    }

    JiWatchedEntry* entry = find_entry(engine, filepath);
    if (!entry) {
        return make_result(JI_HOT_RELOAD_ERROR, "File not watched", filepath, 0, false);
    }

    JiHotReloadResult result = reload_file(engine, entry);

    if (engine->callback) {
        engine->callback(&result, entry->root, engine->callback_user_data);
    }

    return result;
}

JI_API JiObject* ji_hot_reload_get_root(JiHotReloadEngine* engine,
                                           const char* filepath) {
    if (!engine || !filepath) return NULL;
    JiWatchedEntry* entry = find_entry(engine, filepath);
    return entry ? entry->root : NULL;
}

JI_API JiHotReloadResult ji_hot_reload_get_last_error(
    const JiHotReloadEngine* engine,
    const char* filepath) {
    if (!engine || !filepath) {
        return make_result(JI_HOT_RELOAD_OK, "", NULL, 0, false);
    }
    const JiWatchedEntry* entry = find_entry_const(engine, filepath);
    if (!entry) {
        return make_result(JI_HOT_RELOAD_OK, "", NULL, 0, false);
    }
    return entry->last_error;
}

JI_API int ji_hot_reload_watched_count(const JiHotReloadEngine* engine) {
    return engine ? engine->count : 0;
}

/* =========================================================================
 * State Preservation API
 * ========================================================================= */

JI_API bool ji_hot_reload_save_state(JiHotReloadEngine* engine,
                                         JiObject* old_root,
                                         const char* filepath) {
    if (!engine || !old_root || !filepath) return false;

    JiWatchedEntry* entry = find_entry(engine, filepath);
    if (!entry) return false;

    if (!entry->saved_state) {
        entry->saved_state = ji_calloc(1, sizeof(JiSavedState));
        if (!entry->saved_state) return false;
    }

    entry->saved_state->prop_count = 0;
    collect_props_recursive(old_root, entry->saved_state);

    return true;
}

JI_API bool ji_hot_reload_restore_state(JiHotReloadEngine* engine,
                                           JiObject* new_root,
                                           const char* filepath) {
    if (!engine || !new_root || !filepath) return false;

    JiWatchedEntry* entry = find_entry(engine, filepath);
    if (!entry || !entry->saved_state) return false;

    restore_props_recursive(new_root, entry->saved_state);

    return true;
}
