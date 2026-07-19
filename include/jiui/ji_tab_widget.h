/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_tab_widget.h
 * @brief TabWidget — tab bar + stacked content area.
 */

#ifndef JIUI_TAB_WIDGET_H
#define JIUI_TAB_WIDGET_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiTabWidgetPage {
    char*       label;
    JiControl*  content;
    bool        is_closable;
    bool        is_enabled;
} JiTabWidgetPage;

typedef struct JiTabWidget {
    JiControl           control;        /* must be first */
    JiTabWidgetPage*    pages;
    int                 page_count;
    int                 page_capacity;
    int                 current_index;
    int                 tab_height;     /* default 28px */
    bool                tabs_closable;  /* show close buttons on all tabs */
    bool                tabs_movable;   /* tabs can be reordered by drag */
} JiTabWidget;

/** Create a new tab widget. */
JI_API JiTabWidget* ji_tab_widget_new(void);

/** Destroy a tab widget. */
JI_API void ji_tab_widget_destroy(JiTabWidget* tw);

/** Add a tab with label and content widget. Returns the index. */
JI_API int ji_tab_widget_add_tab(JiTabWidget* tw, const char* label, JiControl* content);

/** Insert a tab at the given index. */
JI_API void ji_tab_widget_insert_tab(JiTabWidget* tw, int index, const char* label, JiControl* content);

/** Remove a tab by index. */
JI_API void ji_tab_widget_remove_tab(JiTabWidget* tw, int index);

/** Get the number of tabs. */
JI_API int ji_tab_widget_count(const JiTabWidget* tw);

/** Set the current tab index. */
JI_API void ji_tab_widget_set_current_index(JiTabWidget* tw, int index);

/** Get the current tab index. */
JI_API int ji_tab_widget_get_current_index(const JiTabWidget* tw);

/** Get the content widget for a tab. */
JI_API JiControl* ji_tab_widget_get_content(const JiTabWidget* tw, int index);

/** Set the label for a tab. */
JI_API void ji_tab_widget_set_tab_label(JiTabWidget* tw, int index, const char* label);

/** Get the label for a tab. */
JI_API const char* ji_tab_widget_get_tab_label(const JiTabWidget* tw, int index);

/** Set whether a tab is closable. */
JI_API void ji_tab_widget_set_tab_closable(JiTabWidget* tw, int index, bool closable);

/** Set whether a tab is enabled. */
JI_API void ji_tab_widget_set_tab_enabled(JiTabWidget* tw, int index, bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TAB_WIDGET_H */
