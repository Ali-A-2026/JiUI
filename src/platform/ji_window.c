/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_window.c
 * @brief High-level window implementation — wraps platform backend and
 *        OpenGL context into a single easy-to-use JiWindow object.
 */

/* strdup is POSIX, not C99 */
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/jiui.h"
#include "ji_window_internal.h"
#include "jiui/ji_error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * High-resolution timer
 * ========================================================================= */

static double ji_get_monotonic_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =========================================================================
 * JiWindow implementation
 * ========================================================================= */

JiWindow* ji_window_create(const char* title, int width, int height,
                             JiWindowFlags flags, JiPlatformBackend* backend) {
    if (width <= 0) width = 800;
    if (height <= 0) height = 600;

    /* Get default backend if none specified */
    if (!backend) {
        backend = ji_platform_get_default();
    }
    if (!backend) {
        JI_WARN("No platform backend available");
        return NULL;
    }

    /* Allocate window */
    JiWindow* window = (JiWindow*)calloc(1, sizeof(JiWindow));
    if (!window) return NULL;

    window->backend = backend;
    window->width   = width;
    window->height  = height;
    window->is_open = false;
    window->should_close = false;
    window->frame_count  = 0;
    window->fps          = 0.0;
    window->last_fps_time = ji_get_monotonic_time();
    window->last_frame_time = ji_get_monotonic_time();

    /* Duplicate title */
    window->title = title ? strdup(title) : strdup("JiUI Window");

    /* Create native window */
    window->native_window = backend->create_window(backend, window->title,
                                                     0, 0, width, height);
    if (!window->native_window) {
        JI_WARN("Failed to create native window");
        free(window->title);
        free(window);
        return NULL;
    }

    /* Create OpenGL context if requested */
    if (flags & JI_WINDOW_OPENGL) {
        window->gl_context = backend->create_gl_context(backend, window->native_window);
        if (!window->gl_context) {
            JI_WARN("Failed to create OpenGL context");
        } else {
            /* Make context current */
            if (backend->make_current(backend, window->native_window, window->gl_context)) {
                /* Initialize OpenGL state */
                JiDrawingContext* ctx = backend->get_drawing_context(backend);
                if (ctx && ctx->resize) {
                    ctx->resize(ctx, width, height);
                }

                /* Set swap interval */
                if (flags & JI_WINDOW_ADAPTIVE_VSYNC) {
                    backend->set_swap_interval(backend, -1);  /* adaptive */
                } else if (flags & JI_WINDOW_VSYNC) {
                    backend->set_swap_interval(backend, 1);    /* vsync on */
                } else {
                    backend->set_swap_interval(backend, 0);    /* vsync off = max FPS */
                }

                /* Copy drawing context to window */
                if (ctx) {
                    memcpy(&window->drawing_context, ctx, sizeof(JiDrawingContext));
                }
            }
        }
    }

    /* Initialize drawing context if not already done */
    if (!ji_drawing_context_is_valid(&window->drawing_context)) {
        ji_drawing_context_init(&window->drawing_context);
    }

    window->is_open = true;

    JI_INFO("Window created: %dx%d \"%s\"", width, height, window->title);
    return window;
}

void ji_window_destroy(JiWindow* window) {
    if (!window) return;

    /* Destroy GL context */
    if (window->gl_context && window->backend && window->backend->destroy_gl_context) {
        window->backend->destroy_gl_context(window->backend, window->gl_context);
    }

    /* Destroy native window */
    if (window->native_window && window->backend && window->backend->destroy_window) {
        window->backend->destroy_window(window->backend, window->native_window);
    }

    free(window->title);
    free(window);
}

void ji_window_run(JiWindow* window) {
    if (!window) return;

    while (window->is_open && !window->should_close) {
        if (!ji_window_frame(window)) {
            break;
        }
    }

    window->is_open = false;
}

bool ji_window_frame(JiWindow* window) {
    if (!window || !window->backend) return false;

    /* Poll events */
    JiEvent events[32];
    int count = window->backend->poll_events(window->backend, events, 32);

    for (int i = 0; i < count; i++) {
        JiEvent* ev = &events[i];

        /* Handle resize */
        if (ev->kind == JI_EVENT_RESIZE) {
            window->width  = ev->width;
            window->height = ev->height;

            /* Update GL viewport */
            if (window->drawing_context.resize) {
                window->drawing_context.resize(&window->drawing_context,
                                                ev->width, ev->height);
            }
        }

        /* Handle close */
        if (ev->kind == JI_EVENT_CLOSE) {
            window->should_close = true;
            window->is_open = false;

            if (window->on_close) {
                window->on_close(window, window->user_data);
            }
            return false;
        }

        /* Forward to user callback */
        if (window->on_event) {
            window->on_event(window, ev, window->user_data);
        }
    }

    /* Timing */
    double now = ji_get_monotonic_time();
    window->last_frame_time = now;

    /* FPS calculation — update every 0.5 seconds */
    double fps_elapsed = now - window->last_fps_time;
    if (fps_elapsed >= 0.5) {
        window->fps = (double)window->frame_count / fps_elapsed;
        window->frame_count = 0;
        window->last_fps_time = now;
    }

    /* Make GL context current before rendering */
    if (window->gl_context && window->backend && window->backend->make_current) {
        window->backend->make_current(window->backend, window->native_window, window->gl_context);
    }

    /* Render callback */
    if (window->on_render) {
        window->on_render(window, window->user_data);
    }

    /* Swap buffers */
    if (window->gl_context && window->backend->swap_buffers) {
        window->backend->swap_buffers(window->backend, window->native_window);
    }

    window->frame_count++;
    return true;
}

void ji_window_set_title(JiWindow* window, const char* title) {
    if (!window) return;

    free(window->title);
    window->title = title ? strdup(title) : strdup("JiUI Window");

    if (window->backend && window->backend->set_window_title) {
        window->backend->set_window_title(window->backend,
                                            window->native_window,
                                            window->title);
    }
}

const char* ji_window_get_title(const JiWindow* window) {
    return window ? window->title : NULL;
}

void ji_window_get_size(const JiWindow* window, int* out_width, int* out_height) {
    if (!window) {
        if (out_width)  *out_width = 0;
        if (out_height) *out_height = 0;
        return;
    }
    if (out_width)  *out_width = window->width;
    if (out_height) *out_height = window->height;
}

void ji_window_close(JiWindow* window) {
    if (!window) return;
    window->should_close = true;
    window->is_open = false;
}

bool ji_window_is_open(const JiWindow* window) {
    return window ? window->is_open : false;
}

JiDrawingContext* ji_window_get_drawing_context(JiWindow* window) {
    return window ? &window->drawing_context : NULL;
}

void ji_window_set_render_callback(JiWindow* window,
                                     JiWindowRenderFunc callback,
                                     void* user_data) {
    if (!window) return;
    window->on_render = callback;
    window->user_data = user_data;
}

void ji_window_set_event_callback(JiWindow* window,
                                    JiWindowEventFunc callback,
                                    void* user_data) {
    if (!window) return;
    window->on_event = callback;
    /* Note: user_data is shared across callbacks — set it via render callback */
    (void)user_data;
}

void ji_window_set_close_callback(JiWindow* window,
                                    JiWindowCloseFunc callback,
                                    void* user_data) {
    if (!window) return;
    window->on_close = callback;
    (void)user_data;
}

double ji_window_get_fps(const JiWindow* window) {
    return window ? window->fps : 0.0;
}

uint64_t ji_window_get_frame_count(const JiWindow* window) {
    return window ? window->frame_count : 0;
}

double ji_window_get_last_frame_time(const JiWindow* window) {
    return window ? window->last_frame_time : 0.0;
}

JiPlatformBackend* ji_window_get_backend(const JiWindow* window) {
    return window ? window->backend : NULL;
}

void* ji_window_get_native_window(const JiWindow* window) {
    return window ? window->native_window : NULL;
}

JiGLContext* ji_window_get_gl_context(const JiWindow* window) {
    return window ? window->gl_context : NULL;
}

void* ji_window_get_user_data(const JiWindow* window) {
    return window ? window->user_data : NULL;
}

void ji_window_set_user_data(JiWindow* window, void* user_data) {
    if (!window) return;
    window->user_data = user_data;
}

double ji_window_get_time(void) {
    return ji_get_monotonic_time();
}
