/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_platform.c
 * @brief Platform backend registry — register, find, and manage backends.
 */

#include "jiui/jiui.h"
#include "jiui/ji_platform.h"
#include "jiui/ji_error.h"

#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Backend registry
 * ========================================================================= */

#define JI_MAX_BACKENDS 8

static JiPlatformBackend* g_backends[JI_MAX_BACKENDS];
static int g_backend_count = 0;

/* ---- Public API ---- */

bool ji_platform_register(JiPlatformBackend* backend) {
    if (!backend) return false;
    if (g_backend_count >= JI_MAX_BACKENDS) {
        JI_WARN("Platform backend registry full (max %d)", JI_MAX_BACKENDS);
        return false;
    }
    g_backends[g_backend_count++] = backend;
    JI_INFO("Registered platform backend: %s",
            backend->get_name ? backend->get_name(backend) : "unknown");
    return true;
}

void ji_platform_unregister(JiPlatformBackend* backend) {
    if (!backend) return;
    for (int i = 0; i < g_backend_count; i++) {
        if (g_backends[i] == backend) {
            /* Shift remaining entries */
            for (int j = i; j < g_backend_count - 1; j++) {
                g_backends[j] = g_backends[j + 1];
            }
            g_backend_count--;
            return;
        }
    }
}

JiPlatformBackend* ji_platform_get_default(void) {
    if (g_backend_count > 0) return g_backends[0];
    return NULL;
}

JiPlatformBackend* ji_platform_find(const char* name) {
    if (!name) return NULL;
    for (int i = 0; i < g_backend_count; i++) {
        if (g_backends[i]->get_name) {
            const char* bname = g_backends[i]->get_name(g_backends[i]);
            if (bname && strcmp(bname, name) == 0) {
                return g_backends[i];
            }
        }
    }
    return NULL;
}

void ji_platform_type_init(void) {
    /* Nothing to do currently — backends are registered externally */
}
