/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_frame.h
 * @brief Frame — simple bordered container with frame shape enum.
 */

#ifndef JIUI_FRAME_H
#define JIUI_FRAME_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiFrameShape {
    JI_FRAME_NO_FRAME    = 0,
    JI_FRAME_BOX         = 1,
    JI_FRAME_PANEL       = 2,
    JI_FRAME_STYLED_PANEL = 3,
    JI_FRAME_HLINE       = 4,
    JI_FRAME_VLINE       = 5
} JiFrameShape;

typedef enum JiFrameShadow {
    JI_FRAME_PLAIN   = 0,
    JI_FRAME_RAISED  = 1,
    JI_FRAME_SUNKEN  = 2
} JiFrameShadow;

typedef struct JiFrame {
    JiControl     control;       /* must be first */
    JiFrameShape  shape;
    JiFrameShadow shadow;
    int           line_width;    /* border line width (default 1) */
    int           mid_line_width;
    int           frame_margin;  /* margin between frame and content */
    JiControl*    content;       /* child widget */
} JiFrame;

/** Create a new frame. */
JI_API JiFrame* ji_frame_new(void);

/** Destroy a frame. */
JI_API void ji_frame_destroy(JiFrame* frame);

/** Set the frame shape. */
JI_API void ji_frame_set_shape(JiFrame* frame, JiFrameShape shape);

/** Get the frame shape. */
JI_API JiFrameShape ji_frame_get_shape(const JiFrame* frame);

/** Set the frame shadow. */
JI_API void ji_frame_set_shadow(JiFrame* frame, JiFrameShadow shadow);

/** Get the frame shadow. */
JI_API JiFrameShadow ji_frame_get_shadow(const JiFrame* frame);

/** Set the line width. */
JI_API void ji_frame_set_line_width(JiFrame* frame, int width);

/** Get the line width. */
JI_API int ji_frame_get_line_width(const JiFrame* frame);

/** Set the content widget. */
JI_API void ji_frame_set_content(JiFrame* frame, JiControl* content);

/** Get the content widget. */
JI_API JiControl* ji_frame_get_content(const JiFrame* frame);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_FRAME_H */
