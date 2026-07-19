/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_file_watcher.c
 * @brief Cross-platform file system watcher (polling-based).
 *
 * Uses stat() to detect file modification times. This is a simple,
 * portable approach that works on all platforms without requiring
 * inotify (Linux), FSEvents (macOS), or ReadDirectoryChangesW (Windows).
 *
 * For production use, platform-specific watchers could be added for
 * better performance, but polling is sufficient for development-time
 * hot reload.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_hot_reload.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
  #include <windows.h>
  #define JI_STAT _stat
  #define JI_STAT_STRUCT struct _stat
#else
  #include <unistd.h>
  #define JI_STAT stat
  #define JI_STAT_STRUCT struct stat
#endif

/* =========================================================================
 * File Watcher State
 * ========================================================================= */

typedef struct JiWatchedFile {
    char     filepath[512];     /* Path to the file */
    int64_t  last_mtime;        /* Last modification time (seconds since epoch) */
    int64_t  last_size;        /* Last known file size */
    bool     exists;           /* Whether the file existed last time we checked */
} JiWatchedFile;

struct JiFileWatcher {
    JiWatchedFile  files[JI_FILE_WATCHER_MAX_FILES];
    int            count;
};

/* =========================================================================
 * Helper: get file modification time
 * ========================================================================= */
static int64_t get_file_mtime(const char* filepath, int64_t* out_size) {
    if (out_size) *out_size = 0;
    JI_STAT_STRUCT st;
    if (JI_STAT(filepath, &st) != 0) return 0;

    if (out_size) *out_size = (int64_t)st.st_size;

#if defined(_WIN32)
    return (int64_t)st.st_mtime;
#else
    return (int64_t)st.st_mtime;
#endif
}

/* Get current time in milliseconds */
static int64_t get_current_time_ms(void) {
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (int64_t)(uli.QuadPart / 10000);
#else
    return (int64_t)time(NULL) * 1000;
#endif
}

/* =========================================================================
 * File Watcher API
 * ========================================================================= */

JI_API JiFileWatcher* ji_file_watcher_new(void) {
    JiFileWatcher* w = ji_calloc(1, sizeof(JiFileWatcher));
    return w;
}

JI_API void ji_file_watcher_destroy(JiFileWatcher* watcher) {
    if (!watcher) return;
    ji_free(watcher);
}

JI_API bool ji_file_watcher_add(JiFileWatcher* watcher, const char* filepath) {
    if (!watcher || !filepath) return false;
    if (watcher->count >= JI_FILE_WATCHER_MAX_FILES) return false;

    /* Check if already watching this file */
    for (int i = 0; i < watcher->count; i++) {
        if (strcmp(watcher->files[i].filepath, filepath) == 0) return true;
    }

    JiWatchedFile* wf = &watcher->files[watcher->count];
    strncpy(wf->filepath, filepath, sizeof(wf->filepath) - 1);
    wf->filepath[sizeof(wf->filepath) - 1] = '\0';

    int64_t size = 0;
    int64_t mtime = get_file_mtime(filepath, &size);
    wf->last_mtime = mtime;
    wf->last_size = size;
    wf->exists = (mtime > 0);

    watcher->count++;
    return true;
}

JI_API bool ji_file_watcher_remove(JiFileWatcher* watcher, const char* filepath) {
    if (!watcher || !filepath) return false;

    for (int i = 0; i < watcher->count; i++) {
        if (strcmp(watcher->files[i].filepath, filepath) == 0) {
            /* Shift remaining files down */
            for (int j = i; j < watcher->count - 1; j++) {
                watcher->files[j] = watcher->files[j + 1];
            }
            watcher->count--;
            return true;
        }
    }
    return false;
}

JI_API int ji_file_watcher_poll(JiFileWatcher* watcher,
                                   JiFileWatchCallback callback,
                                   void* user_data) {
    if (!watcher || !callback) return 0;

    int events = 0;

    for (int i = 0; i < watcher->count; i++) {
        JiWatchedFile* wf = &watcher->files[i];

        int64_t size = 0;
        int64_t mtime = get_file_mtime(wf->filepath, &size);
        bool exists = (mtime > 0);

        JiFileNotification notif;
        memset(&notif, 0, sizeof(notif));
        notif.timestamp = get_current_time_ms();
        strncpy(notif.filepath, wf->filepath, sizeof(notif.filepath) - 1);

        if (!wf->exists && exists) {
            /* File was created (or re-appeared) */
            notif.event = JI_FILE_EVENT_CREATED;
            callback(&notif, user_data);
            events++;
        } else if (wf->exists && !exists) {
            /* File was deleted */
            notif.event = JI_FILE_EVENT_DELETED;
            callback(&notif, user_data);
            events++;
        } else if (exists && (mtime != wf->last_mtime || size != wf->last_size)) {
            /* File was modified */
            notif.event = JI_FILE_EVENT_MODIFIED;
            callback(&notif, user_data);
            events++;
        }

        /* Update state */
        wf->last_mtime = mtime;
        wf->last_size = size;
        wf->exists = exists;
    }

    return events;
}

JI_API int ji_file_watcher_count(const JiFileWatcher* watcher) {
    return watcher ? watcher->count : 0;
}
