/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_tool_bar.h
 * @brief ToolBar — movable toolbar with actions, separators, and overflow.
 */

#ifndef JIUI_TOOL_BAR_H
#define JIUI_TOOL_BAR_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiToolBarArea {
    JI_TOOL_BAR_LEFT   = 0,
    JI_TOOL_BAR_RIGHT  = 1,
    JI_TOOL_BAR_TOP    = 2,
    JI_TOOL_BAR_BOTTOM = 3
} JiToolBarArea;

typedef struct JiToolBarItem {
    char*      name;
    char*      label;
    JiControl* widget;       /* NULL for separators */
    bool       is_separator;
} JiToolBarItem;

typedef struct JiToolBar {
    JiControl        control;        /* must be first */
    JiToolBarItem*   items;
    int              item_count;
    int              item_capacity;
    JiToolBarArea    area;
    bool             is_movable;
    int              icon_size;      /* default 24px */
    int              button_style;   /* 0=icon, 1=text, 2=text_beside_icon, 3=text_under_icon */
} JiToolBar;

/** Create a new toolbar. */
JI_API JiToolBar* ji_tool_bar_new(void);

/** Destroy a toolbar. */
JI_API void ji_tool_bar_destroy(JiToolBar* tb);

/** Add an action button. */
JI_API void ji_tool_bar_add_action(JiToolBar* tb, const char* name, const char* label);

/** Add a widget. */
JI_API void ji_tool_bar_add_widget(JiToolBar* tb, JiControl* widget);

/** Add a separator. */
JI_API void ji_tool_bar_add_separator(JiToolBar* tb);

/** Set the toolbar area. */
JI_API void ji_tool_bar_set_area(JiToolBar* tb, JiToolBarArea area);

/** Set whether the toolbar is movable. */
JI_API void ji_tool_bar_set_movable(JiToolBar* tb, bool movable);

/** Set the icon size. */
JI_API void ji_tool_bar_set_icon_size(JiToolBar* tb, int size);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TOOL_BAR_H */
