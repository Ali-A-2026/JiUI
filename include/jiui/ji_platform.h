/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_platform.h
 * @brief Platform backend abstraction — window creation, event loop,
 *        OpenGL context management. Backends (X11, Wayland, etc.) implement
 *        the JiPlatformBackend interface.
 */

#ifndef JIUI_PLATFORM_H
#define JIUI_PLATFORM_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include "ji_drawing.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Event types
 * ========================================================================= */

/** Keyboard key codes — matches X11 keysym values for common keys. */
typedef enum JiKeyCode {
    JI_KEY_UNKNOWN  = 0,
    JI_KEY_ESCAPE   = 0xFF1B,
    JI_KEY_RETURN   = 0xFF0D,
    JI_KEY_TAB      = 0xFF09,
    JI_KEY_BACKSPACE= 0xFF08,
    JI_KEY_DELETE   = 0xFFFF,
    JI_KEY_INSERT   = 0xFF63,
    JI_KEY_HOME     = 0xFF50,
    JI_KEY_END      = 0xFF57,
    JI_KEY_PAGE_UP  = 0xFF55,
    JI_KEY_PAGE_DOWN= 0xFF56,
    JI_KEY_LEFT     = 0xFF51,
    JI_KEY_RIGHT    = 0xFF53,
    JI_KEY_UP       = 0xFF52,
    JI_KEY_DOWN     = 0xFF54,
    JI_KEY_SHIFT_L  = 0xFFE1,
    JI_KEY_SHIFT_R  = 0xFFE2,
    JI_KEY_CONTROL_L= 0xFFE3,
    JI_KEY_CONTROL_R= 0xFFE4,
    JI_KEY_ALT_L    = 0xFFE9,
    JI_KEY_ALT_R    = 0xFFEA,
    JI_KEY_F1       = 0xFFBE,
    JI_KEY_F2       = 0xFFBF,
    JI_KEY_F3       = 0xFFC0,
    JI_KEY_F4       = 0xFFC1,
    JI_KEY_F5       = 0xFFC2,
    JI_KEY_F6       = 0xFFC3,
    JI_KEY_F7       = 0xFFC4,
    JI_KEY_F8       = 0xFFC5,
    JI_KEY_F9       = 0xFFC6,
    JI_KEY_F10      = 0xFFC7,
    JI_KEY_F11      = 0xFFC8,
    JI_KEY_F12      = 0xFFC9,
    JI_KEY_SPACE    = 0x0020,
    JI_KEY_A        = 0x0061,
    JI_KEY_B        = 0x0062,
    JI_KEY_C        = 0x0063,
    JI_KEY_D        = 0x0064,
    JI_KEY_E        = 0x0065,
    JI_KEY_F        = 0x0066,
    JI_KEY_G        = 0x0067,
    JI_KEY_H        = 0x0068,
    JI_KEY_I        = 0x0069,
    JI_KEY_J        = 0x006A,
    JI_KEY_K        = 0x006B,
    JI_KEY_L        = 0x006C,
    JI_KEY_M        = 0x006D,
    JI_KEY_N        = 0x006E,
    JI_KEY_O        = 0x006F,
    JI_KEY_P        = 0x0070,
    JI_KEY_Q        = 0x0071,
    JI_KEY_R        = 0x0072,
    JI_KEY_S        = 0x0073,
    JI_KEY_T        = 0x0074,
    JI_KEY_U        = 0x0075,
    JI_KEY_V        = 0x0076,
    JI_KEY_W        = 0x0077,
    JI_KEY_X        = 0x0078,
    JI_KEY_Y        = 0x0079,
    JI_KEY_Z        = 0x007A,
} JiKeyCode;

/** Mouse button indices. */
typedef enum JiMouseButton {
    JI_MOUSE_NONE    = 0,
    JI_MOUSE_LEFT    = 1,
    JI_MOUSE_MIDDLE  = 2,
    JI_MOUSE_RIGHT   = 3,
    JI_MOUSE_4       = 4,
    JI_MOUSE_5       = 5,
} JiMouseButton;

/** Event kind discriminant. */
typedef enum JiEventKind {
    JI_EVENT_NONE        = 0,
    JI_EVENT_KEY_PRESS   = 1,
    JI_EVENT_KEY_RELEASE = 2,
    JI_EVENT_MOUSE_MOVE  = 3,
    JI_EVENT_MOUSE_PRESS = 4,
    JI_EVENT_MOUSE_RELEASE= 5,
    JI_EVENT_MOUSE_WHEEL = 6,
    JI_EVENT_RESIZE      = 7,
    JI_EVENT_CLOSE       = 8,
    JI_EVENT_FOCUS_IN    = 9,
    JI_EVENT_FOCUS_OUT   = 10,
} JiEventKind;

/** Modifier key flags. */
typedef enum JiModifier {
    JI_MOD_SHIFT   = 1 << 0,
    JI_MOD_CONTROL = 1 << 1,
    JI_MOD_ALT     = 1 << 2,
    JI_MOD_SUPER   = 1 << 3,
} JiModifier;

/** A single platform event. */
typedef struct JiEvent {
    JiEventKind kind;

    /* Keyboard */
    JiKeyCode key;
    uint32_t  modifiers;   /* JiModifier bitmask */

    /* Mouse */
    JiMouseButton button;
    double mouse_x;
    double mouse_y;
    double wheel_delta;     /* positive = scroll up */

    /* Resize */
    int width;
    int height;
} JiEvent;

/* =========================================================================
 * JiGLContext — OpenGL rendering context
 * ========================================================================= */

/** Opaque OpenGL context handle. */
typedef struct JiGLContext JiGLContext;

/* =========================================================================
 * JiPlatformBackend — interface for platform-specific operations
 * ========================================================================= */

/** Forward declaration. */
typedef struct JiPlatformBackend JiPlatformBackend;

/** Create a native window. Returns opaque handle. */
typedef void* (*JiPlatformCreateWindowFunc)(JiPlatformBackend* backend,
                                             const char* title,
                                             int x, int y, int width, int height);

/** Destroy a native window. */
typedef void (*JiPlatformDestroyWindowFunc)(JiPlatformBackend* backend, void* native_window);

/** Show the window. */
typedef void (*JiPlatformShowWindowFunc)(JiPlatformBackend* backend, void* native_window);

/** Hide the window. */
typedef void (*JiPlatformHideWindowFunc)(JiPlatformBackend* backend, void* native_window);

/** Set the window title. */
typedef void (*JiPlatformSetWindowTitleFunc)(JiPlatformBackend* backend,
                                              void* native_window,
                                              const char* title);

/** Get the window size in pixels. */
typedef void (*JiPlatformGetWindowSizeFunc)(JiPlatformBackend* backend,
                                             void* native_window,
                                             int* out_width, int* out_height);

/** Poll for events (non-blocking). Returns number of events read. */
typedef int (*JiPlatformPollEventsFunc)(JiPlatformBackend* backend,
                                         JiEvent* out_events, int max_events);

/** Wait for at least one event (blocking). Returns number of events read. */
typedef int (*JiPlatformWaitEventsFunc)(JiPlatformBackend* backend,
                                         JiEvent* out_events, int max_events);

/** Check if the window should close. */
typedef bool (*JiPlatformShouldCloseFunc)(JiPlatformBackend* backend, void* native_window);

/** Request the window to close. */
typedef void (*JiPlatformRequestCloseFunc)(JiPlatformBackend* backend, void* native_window);

/** Create an OpenGL context for the given window. */
typedef JiGLContext* (*JiPlatformCreateGLContextFunc)(JiPlatformBackend* backend,
                                                        void* native_window);

/** Destroy an OpenGL context. */
typedef void (*JiPlatformDestroyGLContextFunc)(JiPlatformBackend* backend, JiGLContext* ctx);

/** Make the GL context current for the calling thread. */
typedef bool (*JiPlatformMakeCurrentFunc)(JiPlatformBackend* backend,
                                            void* native_window,
                                            JiGLContext* ctx);

/** Swap buffers (present). */
typedef void (*JiPlatformSwapBuffersFunc)(JiPlatformBackend* backend,
                                            void* native_window);

/** Set swap interval (0 = no vsync, 1 = vsync, -1 = adaptive). */
typedef void (*JiPlatformSetSwapIntervalFunc)(JiPlatformBackend* backend, int interval);

/** Get the drawing context for this backend. */
typedef JiDrawingContext* (*JiPlatformGetDrawingContextFunc)(JiPlatformBackend* backend);

/** Destroy the platform backend and free all resources. */
typedef void (*JiPlatformDestroyBackendFunc)(JiPlatformBackend* backend);

/** Get the backend name (e.g. "X11", "Wayland"). */
typedef const char* (*JiPlatformGetNameFunc)(JiPlatformBackend* backend);

/** Get the native display connection (for interop). */
typedef void* (*JiPlatformGetNativeDisplayFunc)(JiPlatformBackend* backend);

/** Get the native window handle (for interop). */
typedef void* (*JiPlatformGetNativeWindowFunc)(JiPlatformBackend* backend, void* native_window);

/**
 * Platform backend interface — fill in all function pointers to
 * create a new backend (X11, Wayland, etc.).
 */
struct JiPlatformBackend {
    JiPlatformCreateWindowFunc      create_window;
    JiPlatformDestroyWindowFunc     destroy_window;
    JiPlatformShowWindowFunc        show_window;
    JiPlatformHideWindowFunc        hide_window;
    JiPlatformSetWindowTitleFunc    set_window_title;
    JiPlatformGetWindowSizeFunc     get_window_size;
    JiPlatformPollEventsFunc        poll_events;
    JiPlatformWaitEventsFunc        wait_events;
    JiPlatformShouldCloseFunc       should_close;
    JiPlatformRequestCloseFunc      request_close;
    JiPlatformCreateGLContextFunc   create_gl_context;
    JiPlatformDestroyGLContextFunc  destroy_gl_context;
    JiPlatformMakeCurrentFunc       make_current;
    JiPlatformSwapBuffersFunc       swap_buffers;
    JiPlatformSetSwapIntervalFunc   set_swap_interval;
    JiPlatformGetDrawingContextFunc get_drawing_context;
    JiPlatformDestroyBackendFunc    destroy_backend;
    JiPlatformGetNameFunc           get_name;
    JiPlatformGetNativeDisplayFunc  get_native_display;
    JiPlatformGetNativeWindowFunc   get_native_window;

    /* Backend-specific data */
    void* backend_data;
};

/* ---- Backend registry ---- */

/** Register a platform backend. Returns true on success. */
JI_API bool ji_platform_register(JiPlatformBackend* backend);

/** Unregister a platform backend. */
JI_API void ji_platform_unregister(JiPlatformBackend* backend);

/** Get the default platform backend (first registered). */
JI_API JiPlatformBackend* ji_platform_get_default(void);

/** Find a backend by name. */
JI_API JiPlatformBackend* ji_platform_find(const char* name);

/** Initialize the platform subsystem. Called by ji_initialize(). */
JI_API void ji_platform_type_init(void);

/* ---- Built-in backend constructors ---- */

#ifdef JIUI_ENABLE_X11
/** Create and register the X11+GLX platform backend. Returns NULL on failure. */
JI_API JiPlatformBackend* ji_x11_backend_create(void);
#endif

#ifdef JIUI_ENABLE_WAYLAND
/** Create and register the Wayland platform backend. Returns NULL on failure. */
JI_API JiPlatformBackend* ji_wayland_backend_create(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PLATFORM_H */
