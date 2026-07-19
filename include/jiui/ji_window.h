/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_window.h
 * @brief High-level window abstraction — wraps platform backend and
 *        OpenGL context into a single easy-to-use JiWindow object.
 */

#ifndef JIUI_WINDOW_H
#define JIUI_WINDOW_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_platform.h"
#include "ji_drawing.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Window creation hints
 * ========================================================================= */

typedef enum JiWindowFlags {
    JI_WINDOW_NONE          = 0,
    JI_WINDOW_RESIZABLE     = 1 << 0,
    JI_WINDOW_FULLSCREEN    = 1 << 1,
    JI_WINDOW_BORDERLESS    = 1 << 2,
    JI_WINDOW_OPENGL        = 1 << 3,   /* request OpenGL context */
    JI_WINDOW_VSYNC         = 1 << 4,   /* enable vsync */
    JI_WINDOW_ADAPTIVE_VSYNC= 1 << 5,   /* adaptive vsync (if supported) */
} JiWindowFlags;

/* =========================================================================
 * JiWindow — high-level window object
 * ========================================================================= */

/** Opaque window object — use accessor functions to query/set properties. */
typedef struct JiWindow JiWindow;

/** Callback: called every frame for rendering. */
typedef void (*JiWindowRenderFunc)(JiWindow* window, void* user_data);

/** Callback: called when an event is received. */
typedef void (*JiWindowEventFunc)(JiWindow* window, const JiEvent* event, void* user_data);

/** Callback: called when the window is closing. */
typedef void (*JiWindowCloseFunc)(JiWindow* window, void* user_data);

/* ---- Lifetime ---- */

/**
 * Create a new window.
 * @param title    Window title (UTF-8)
 * @param width    Initial width in pixels
 * @param height   Initial height in pixels
 * @param flags    JiWindowFlags bitmask
 * @param backend  Platform backend to use (NULL = default)
 * @return New window, or NULL on failure
 */
JI_API JiWindow* ji_window_create(const char* title, int width, int height,
                                    JiWindowFlags flags, JiPlatformBackend* backend);

/** Destroy a window and free all resources. */
JI_API void ji_window_destroy(JiWindow* window);

/* ---- Main loop ---- */

/** Run the window's event/render loop until closed. */
JI_API void ji_window_run(JiWindow* window);

/** Process a single frame: poll events, call on_render, swap buffers. */
JI_API bool ji_window_frame(JiWindow* window);

/* ---- Properties ---- */

/** Set the window title. */
JI_API void ji_window_set_title(JiWindow* window, const char* title);

/** Get the window title. */
JI_API const char* ji_window_get_title(const JiWindow* window);

/** Get the window size in pixels. */
JI_API void ji_window_get_size(const JiWindow* window, int* out_width, int* out_height);

/** Request the window to close. */
JI_API void ji_window_close(JiWindow* window);

/** Check if the window is still open. */
JI_API bool ji_window_is_open(const JiWindow* window);

/** Get the drawing context. */
JI_API JiDrawingContext* ji_window_get_drawing_context(JiWindow* window);

/** Get the platform backend. */
JI_API JiPlatformBackend* ji_window_get_backend(const JiWindow* window);

/** Get the native window handle (e.g. X11 Window, Wayland surface). */
JI_API void* ji_window_get_native_window(const JiWindow* window);

/** Get the OpenGL context. */
JI_API JiGLContext* ji_window_get_gl_context(const JiWindow* window);

/** Get the user data pointer. */
JI_API void* ji_window_get_user_data(const JiWindow* window);

/** Set the user data pointer. */
JI_API void ji_window_set_user_data(JiWindow* window, void* user_data);

/* ---- Callbacks ---- */

/** Set the render callback. */
JI_API void ji_window_set_render_callback(JiWindow* window,
                                            JiWindowRenderFunc callback,
                                            void* user_data);

/** Set the event callback. */
JI_API void ji_window_set_event_callback(JiWindow* window,
                                           JiWindowEventFunc callback,
                                           void* user_data);

/** Set the close callback. */
JI_API void ji_window_set_close_callback(JiWindow* window,
                                           JiWindowCloseFunc callback,
                                           void* user_data);

/* ---- Timing / FPS ---- */

/** Get the current FPS (frames per second). */
JI_API double ji_window_get_fps(const JiWindow* window);

/** Get the total frame count since window creation. */
JI_API uint64_t ji_window_get_frame_count(const JiWindow* window);

/** Get the last frame time in seconds. */
JI_API double ji_window_get_last_frame_time(const JiWindow* window);

/** Get the current time in seconds (monotonic high-resolution). */
JI_API double ji_window_get_time(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_WINDOW_H */
