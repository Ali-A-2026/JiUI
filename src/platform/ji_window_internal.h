/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_window_internal.h
 * @brief Internal definition of the JiWindow struct.
 *        This header is NOT part of the public API — it is only included
 *        by implementation files that need direct access to struct fields.
 */

#ifndef JIUI_WINDOW_INTERNAL_H
#define JIUI_WINDOW_INTERNAL_H

#include "jiui/ji_window.h"
#include "jiui/ji_platform.h"
#include "jiui/ji_drawing.h"
#include <stdbool.h>
#include <stdint.h>

struct JiWindow {
    /* Platform */
    JiPlatformBackend* backend;
    void*              native_window;
    JiGLContext*       gl_context;

    /* Drawing */
    JiDrawingContext   drawing_context;

    /* State */
    bool               is_open;
    bool               should_close;
    int                width;
    int                height;
    char*              title;

    /* Callbacks */
    JiWindowRenderFunc on_render;
    JiWindowEventFunc  on_event;
    JiWindowCloseFunc  on_close;
    void*              user_data;

    /* Timing */
    uint64_t           frame_count;
    double             fps;
    double             last_fps_time;
    double             last_frame_time;
};

#endif /* JIUI_WINDOW_INTERNAL_H */
