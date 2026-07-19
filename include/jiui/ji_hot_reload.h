/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_hot_reload.h
 * @brief Hot reload engine for JiML files.
 *
 * Watches .jiml files for changes and performs live updates:
 *   - File system watcher (polling-based, cross-platform)
 *   - Parse → diff → patch widget tree
 *   - State preservation across reloads
 *   - Error reporting with overlay support
 *   - Callback-based notification system
 *
 * The hot reload engine is designed to be non-intrusive:
 * it only activates when explicitly enabled, and falls back
 * gracefully when file watching is unavailable.
 */

#ifndef JIUI_HOT_RELOAD_H
#define JIUI_HOT_RELOAD_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_jiml.h"
#include "ji_object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * File Watcher
 * ========================================================================= */

/** Maximum number of files a single watcher can monitor. */
#define JI_FILE_WATCHER_MAX_FILES 64

/** File change event types. */
typedef enum JiFileEvent {
    JI_FILE_EVENT_NONE = 0,
    JI_FILE_EVENT_MODIFIED,     /* File content changed */
    JI_FILE_EVENT_CREATED,      /* New file appeared */
    JI_FILE_EVENT_DELETED,      /* File was removed */
    JI_FILE_EVENT_RENAMED       /* File was renamed/moved */
} JiFileEvent;

/** A file change notification. */
typedef struct JiFileNotification {
    JiFileEvent  event;         /* Type of event */
    char         filepath[512]; /* Path to the changed file */
    int64_t      timestamp;     /* When the event was detected (ms) */
} JiFileNotification;

/** File watcher state (opaque). */
typedef struct JiFileWatcher JiFileWatcher;

/** Callback for file change notifications. */
typedef void (*JiFileWatchCallback)(const JiFileNotification* notif,
                                      void* user_data);

/**
 * Create a new file watcher.
 * @return  New file watcher (caller must destroy with ji_file_watcher_destroy).
 */
JI_API JiFileWatcher* ji_file_watcher_new(void);

/**
 * Destroy a file watcher and free all resources.
 * @param watcher  File watcher to destroy (may be NULL).
 */
JI_API void ji_file_watcher_destroy(JiFileWatcher* watcher);

/**
 * Add a file to the watch list.
 * @param watcher   File watcher.
 * @param filepath  Path to the file to watch.
 * @return         True on success, false if max files reached or file not found.
 */
JI_API bool ji_file_watcher_add(JiFileWatcher* watcher, const char* filepath);

/**
 * Remove a file from the watch list.
 * @param watcher   File watcher.
 * @param filepath  Path to the file to stop watching.
 * @return         True on success, false if file was not being watched.
 */
JI_API bool ji_file_watcher_remove(JiFileWatcher* watcher, const char* filepath);

/**
 * Poll for file changes. Call this periodically (e.g. in an event loop).
 * @param watcher    File watcher.
 * @param callback   Callback to invoke for each detected change.
 * @param user_data  User data passed to the callback.
 * @return          Number of events detected.
 */
JI_API int ji_file_watcher_poll(JiFileWatcher* watcher,
                                   JiFileWatchCallback callback,
                                   void* user_data);

/**
 * Get the number of files currently being watched.
 * @param watcher  File watcher.
 * @return        Number of watched files.
 */
JI_API int ji_file_watcher_count(const JiFileWatcher* watcher);

/* =========================================================================
 * Hot Reload Engine
 * ========================================================================= */

/** Hot reload error severity. */
typedef enum JiHotReloadErrorLevel {
    JI_HOT_RELOAD_OK = 0,       /* No error */
    JI_HOT_RELOAD_WARNING,      /* Warning (reload succeeded with issues) */
    JI_HOT_RELOAD_ERROR,        /* Error (reload failed) */
    JI_HOT_RELOAD_PARSE_ERROR   /* Parse error in .jiml file */
} JiHotReloadErrorLevel;

/** Hot reload result. */
typedef struct JiHotReloadResult {
    JiHotReloadErrorLevel  level;        /* Error level */
    char                   message[512]; /* Error/warning message */
    char                   filepath[512]; /* File that caused the error */
    int                    line;          /* Error line (if applicable) */
    bool                   reloaded;      /* True if reload actually happened */
} JiHotReloadResult;

/** Hot reload engine state (opaque). */
typedef struct JiHotReloadEngine JiHotReloadEngine;

/** Callback invoked when the widget tree is reloaded. */
typedef void (*JiHotReloadCallback)(const JiHotReloadResult* result,
                                      JiObject* new_root,
                                      void* user_data);

/**
 * Create a new hot reload engine.
 * @return  New hot reload engine (caller must destroy with ji_hot_reload_destroy).
 */
JI_API JiHotReloadEngine* ji_hot_reload_new(void);

/**
 * Destroy a hot reload engine and free all resources.
 * @param engine  Hot reload engine to destroy (may be NULL).
 */
JI_API void ji_hot_reload_destroy(JiHotReloadEngine* engine);

/**
 * Register a .jiml file for hot reload.
 * @param engine    Hot reload engine.
 * @param filepath  Path to the .jiml file.
 * @param context   Data context for binding resolution (may be NULL).
 * @return         True on success, false on error.
 */
JI_API bool ji_hot_reload_watch(JiHotReloadEngine* engine,
                                  const char* filepath,
                                  void* context);

/**
 * Unregister a .jiml file from hot reload.
 * @param engine    Hot reload engine.
 * @param filepath  Path to the .jiml file.
 * @return         True on success, false if file was not registered.
 */
JI_API bool ji_hot_reload_unwatch(JiHotReloadEngine* engine,
                                     const char* filepath);

/**
 * Set the callback to invoke when a reload occurs.
 * @param engine     Hot reload engine.
 * @param callback   Callback function (may be NULL to clear).
 * @param user_data  User data passed to the callback.
 */
JI_API void ji_hot_reload_set_callback(JiHotReloadEngine* engine,
                                         JiHotReloadCallback callback,
                                         void* user_data);

/**
 * Enable or disable the hot reload engine.
 * When disabled, polling does nothing.
 * @param engine   Hot reload engine.
 * @param enabled  True to enable, false to disable.
 */
JI_API void ji_hot_reload_set_enabled(JiHotReloadEngine* engine, bool enabled);

/**
 * Check if the hot reload engine is enabled.
 * @param engine  Hot reload engine.
 * @return      True if enabled.
 */
JI_API bool ji_hot_reload_is_enabled(const JiHotReloadEngine* engine);

/**
 * Poll for file changes and trigger reloads if needed.
 * Call this periodically in your event loop (e.g. once per frame).
 * @param engine  Hot reload engine.
 * @return        Number of files reloaded.
 */
JI_API int ji_hot_reload_poll(JiHotReloadEngine* engine);

/**
 * Force a reload of all watched files, regardless of changes.
 * @param engine  Hot reload engine.
 * @return       Number of files reloaded.
 */
JI_API int ji_hot_reload_reload_all(JiHotReloadEngine* engine);

/**
 * Force a reload of a specific file.
 * @param engine    Hot reload engine.
 * @param filepath  Path to the .jiml file.
 * @return         Hot reload result.
 */
JI_API JiHotReloadResult ji_hot_reload_reload(JiHotReloadEngine* engine,
                                                 const char* filepath);

/**
 * Get the current root object for a watched file.
 * @param engine    Hot reload engine.
 * @param filepath  Path to the .jiml file.
 * @return         Root object, or NULL if not loaded yet.
 *                The caller should NOT destroy this object.
 */
JI_API JiObject* ji_hot_reload_get_root(JiHotReloadEngine* engine,
                                          const char* filepath);

/**
 * Get the last error for a watched file.
 * @param engine    Hot reload engine.
 * @param filepath  Path to the .jiml file.
 * @return         Last error result (level=JI_HOT_RELOAD_OK if no error).
 */
JI_API JiHotReloadResult ji_hot_reload_get_last_error(
    const JiHotReloadEngine* engine,
    const char* filepath);

/**
 * Get the number of watched files.
 * @param engine  Hot reload engine.
 * @return      Number of watched files.
 */
JI_API int ji_hot_reload_watched_count(const JiHotReloadEngine* engine);

/* =========================================================================
 * State Preservation
 * ========================================================================= */

/**
 * Save state from an old widget tree before a reload.
 * This captures property values that should survive across reloads.
 * @param engine    Hot reload engine.
 * @param old_root  Old root object (about to be replaced).
 * @param filepath  Path to the .jiml file.
 * @return         True on success.
 */
JI_API bool ji_hot_reload_save_state(JiHotReloadEngine* engine,
                                        JiObject* old_root,
                                        const char* filepath);

/**
 * Restore saved state to a new widget tree after a reload.
 * @param engine    Hot reload engine.
 * @param new_root  New root object (just created).
 * @param filepath  Path to the .jiml file.
 * @return         True on success.
 */
JI_API bool ji_hot_reload_restore_state(JiHotReloadEngine* engine,
                                          JiObject* new_root,
                                          const char* filepath);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_HOT_RELOAD_H */
