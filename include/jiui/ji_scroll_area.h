/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_scroll_area.h
 * @brief ScrollArea — viewport with scrollbars and content widget.
 */

#ifndef JIUI_SCROLL_AREA_H
#define JIUI_SCROLL_AREA_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiScrollPolicy {
    JI_SCROLL_POLICY_OFF        = 0,
    JI_SCROLL_POLICY_ON         = 1,
    JI_SCROLL_POLICY_AS_NEEDED  = 2
} JiScrollPolicy;

typedef struct JiScrollArea {
    JiControl       control;        /* must be first */
    JiControl*      content;        /* the widget being scrolled */
    JiScrollPolicy  h_policy;       /* horizontal scrollbar policy */
    JiScrollPolicy  v_policy;       /* vertical scrollbar policy */
    int             scroll_x;       /* horizontal scroll offset */
    int             scroll_y;       /* vertical scroll offset */
    int             content_width;  /* total content width */
    int             content_height;  /* total content height */
    int             viewport_x;     /* viewport origin x */
    int             viewport_y;     /* viewport origin y */
    int             viewport_width; /* visible width */
    int             viewport_height;/* visible height */
    int             scrollbar_width;/* scrollbar thickness (default 16) */
    bool            widget_resizable;/* auto-resize content to viewport */
} JiScrollArea;

/** Create a new scroll area. */
JI_API JiScrollArea* ji_scroll_area_new(void);

/** Destroy a scroll area. */
JI_API void ji_scroll_area_destroy(JiScrollArea* area);

/** Set the content widget. */
JI_API void ji_scroll_area_set_content(JiScrollArea* area, JiControl* content);

/** Get the content widget. */
JI_API JiControl* ji_scroll_area_get_content(const JiScrollArea* area);

/** Set scrollbar policies. */
JI_API void ji_scroll_area_set_policy(JiScrollArea* area, JiScrollPolicy h, JiScrollPolicy v);

/** Set scroll position. */
JI_API void ji_scroll_area_set_scroll(JiScrollArea* area, int x, int y);

/** Get scroll position. */
JI_API void ji_scroll_area_get_scroll(const JiScrollArea* area, int* x, int* y);

/** Set content size. */
JI_API void ji_scroll_area_set_content_size(JiScrollArea* area, int width, int height);

/** Set whether the content widget is resizable. */
JI_API void ji_scroll_area_set_widget_resizable(JiScrollArea* area, bool resizable);

/** Check if the content widget is resizable. */
JI_API bool ji_scroll_area_is_widget_resizable(const JiScrollArea* area);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SCROLL_AREA_H */
