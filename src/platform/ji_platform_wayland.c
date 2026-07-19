/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * Wayland Platform Backend
 */

#include "jiui/ji_platform.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>

/* Wayland types - opaque, loaded dynamically */
typedef struct wl_display wl_display;
typedef struct wl_registry wl_registry;
typedef struct wl_compositor wl_compositor;
typedef struct wl_surface wl_surface;

/* Wayland backend state */
typedef struct JiWaylandData {
    wl_display*   display;
    wl_surface*   surface;
    uint32_t      width;
    uint32_t      height;
    int           configured;
    int           closed;
    int           has_display;
} JiWaylandData;

/* ---- Check Wayland availability ---- */
static int ji_wayland_available(void) {
    const char* wd = getenv("WAYLAND_DISPLAY");
    if (wd && wd[0] != '\0') return 1;
    const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (runtime_dir) {
        char path[512];
        snprintf(path, sizeof(path), "%s/wayland-0", runtime_dir);
        struct stat st;
        if (stat(path, &st) == 0) return 1;
    }
    return 0;
}

/* ---- Backend function implementations ---- */

static void* ji_wayland_create_window(JiPlatformBackend* backend,
                                       const char* title, int x, int y, int width, int height) {
    (void)backend; (void)title; (void)x; (void)y;
    JiWaylandData* data = ji_calloc(1, sizeof(JiWaylandData));
    if (!data) return NULL;
    data->width = (uint32_t)width;
    data->height = (uint32_t)height;
    data->configured = 0;
    data->closed = 0;
    data->has_display = 0;

    /* Try to connect to Wayland display */
    if (ji_wayland_available()) {
        /* Load libwayland-client and connect */
        void* lib = dlopen("libwayland-client.so.0", RTLD_LAZY);
        if (!lib) lib = dlopen("libwayland-client.so", RTLD_LAZY);
        if (lib) {
            typedef wl_display* (*wl_display_connect_fn)(const char*);
            wl_display_connect_fn connect_fn = (wl_display_connect_fn)dlsym(lib, "wl_display_connect");
            if (connect_fn) {
                data->display = connect_fn(NULL);
                if (data->display) {
                    data->has_display = 1;
                    fprintf(stdout, "[JiUI] Wayland: connected to display\n");
                }
            }
        }
    }

    if (!data->has_display) {
        fprintf(stderr, "[JiUI] Wayland: could not connect to display\n");
    }

    return data;
}

static void ji_wayland_destroy_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend;
    if (native_window) ji_free(native_window);
}

static void ji_wayland_show_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void ji_wayland_hide_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void ji_wayland_set_window_title(JiPlatformBackend* backend, void* native_window, const char* title) {
    (void)backend; (void)native_window; (void)title;
}

static void ji_wayland_get_window_size(JiPlatformBackend* backend, void* native_window, int* out_w, int* out_h) {
    (void)backend;
    if (native_window) {
        JiWaylandData* data = (JiWaylandData*)native_window;
        if (out_w) *out_w = (int)data->width;
        if (out_h) *out_h = (int)data->height;
    }
}

static int ji_wayland_poll_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    (void)backend; (void)out_events; (void)max_events;
    return 0;
}

static int ji_wayland_wait_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    (void)backend; (void)out_events; (void)max_events;
    return 0;
}

static bool ji_wayland_should_close(JiPlatformBackend* backend, void* native_window) {
    (void)backend;
    if (!native_window) return true;
    return ((JiWaylandData*)native_window)->closed != 0;
}

static void ji_wayland_request_close(JiPlatformBackend* backend, void* native_window) {
    (void)backend;
    if (native_window) ((JiWaylandData*)native_window)->closed = 1;
}

static JiGLContext* ji_wayland_create_gl_context(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
    return NULL; /* TODO: EGL context creation */
}

static void ji_wayland_destroy_gl_context(JiPlatformBackend* backend, JiGLContext* ctx) {
    (void)backend; (void)ctx;
}

static bool ji_wayland_make_current(JiPlatformBackend* backend, void* native_window, JiGLContext* ctx) {
    (void)backend; (void)native_window; (void)ctx;
    return false;
}

static void ji_wayland_swap_buffers(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void ji_wayland_set_swap_interval(JiPlatformBackend* backend, int interval) {
    (void)backend; (void)interval;
}

static JiDrawingContext* ji_wayland_get_drawing_context(JiPlatformBackend* backend) {
    (void)backend;
    return NULL;
}

static void ji_wayland_destroy_backend(JiPlatformBackend* backend) {
    if (backend) ji_free(backend);
}

static const char* ji_wayland_get_name(JiPlatformBackend* backend) {
    (void)backend;
    return "Wayland";
}

static void* ji_wayland_get_native_display(JiPlatformBackend* backend) {
    (void)backend;
    return NULL; /* TODO: return wl_display* */
}

static void* ji_wayland_get_native_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
    return NULL; /* TODO: return wl_surface* */
}

/* ---- Public constructor ---- */

JiPlatformBackend* ji_wayland_backend_create(void) {
    if (!ji_wayland_available()) {
        fprintf(stderr, "[JiUI] Wayland: not available\n");
        return NULL;
    }

    JiPlatformBackend* backend = ji_calloc(1, sizeof(JiPlatformBackend));
    if (!backend) return NULL;

    backend->create_window      = ji_wayland_create_window;
    backend->destroy_window     = ji_wayland_destroy_window;
    backend->show_window        = ji_wayland_show_window;
    backend->hide_window        = ji_wayland_hide_window;
    backend->set_window_title   = ji_wayland_set_window_title;
    backend->get_window_size    = ji_wayland_get_window_size;
    backend->poll_events        = ji_wayland_poll_events;
    backend->wait_events        = ji_wayland_wait_events;
    backend->should_close       = ji_wayland_should_close;
    backend->request_close      = ji_wayland_request_close;
    backend->create_gl_context  = ji_wayland_create_gl_context;
    backend->destroy_gl_context = ji_wayland_destroy_gl_context;
    backend->make_current       = ji_wayland_make_current;
    backend->swap_buffers       = ji_wayland_swap_buffers;
    backend->set_swap_interval  = ji_wayland_set_swap_interval;
    backend->get_drawing_context = ji_wayland_get_drawing_context;
    backend->destroy_backend    = ji_wayland_destroy_backend;
    backend->get_name           = ji_wayland_get_name;
    backend->get_native_display = ji_wayland_get_native_display;
    backend->get_native_window  = ji_wayland_get_native_window;
    backend->backend_data       = NULL;

    ji_platform_register(backend);
    fprintf(stdout, "[JiUI] Wayland backend registered\n");
    return backend;
}
