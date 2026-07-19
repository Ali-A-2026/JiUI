/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_stacked_widget.h
 * @brief StackedWidget — page stack with index-based switching.
 */

#ifndef JIUI_STACKED_WIDGET_H
#define JIUI_STACKED_WIDGET_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiStackedWidget {
    JiControl      control;        /* must be first */
    JiControl**    pages;
    int            page_count;
    int            page_capacity;
    int            current_index;
} JiStackedWidget;

/** Create a new stacked widget. */
JI_API JiStackedWidget* ji_stacked_widget_new(void);

/** Destroy a stacked widget. */
JI_API void ji_stacked_widget_destroy(JiStackedWidget* sw);

/** Add a page. Returns the index. */
JI_API int ji_stacked_widget_add_page(JiStackedWidget* sw, JiControl* page);

/** Insert a page at the given index. */
JI_API void ji_stacked_widget_insert_page(JiStackedWidget* sw, int index, JiControl* page);

/** Remove a page by index. */
JI_API void ji_stacked_widget_remove_page(JiStackedWidget* sw, int index);

/** Get the number of pages. */
JI_API int ji_stacked_widget_count(const JiStackedWidget* sw);

/** Set the current page index. */
JI_API void ji_stacked_widget_set_current_index(JiStackedWidget* sw, int index);

/** Get the current page index. */
JI_API int ji_stacked_widget_get_current_index(const JiStackedWidget* sw);

/** Get the current page widget. */
JI_API JiControl* ji_stacked_widget_get_current_page(const JiStackedWidget* sw);

/** Get a page by index. */
JI_API JiControl* ji_stacked_widget_get_page(const JiStackedWidget* sw, int index);

/** Find the index of a page. Returns -1 if not found. */
JI_API int ji_stacked_widget_index_of(const JiStackedWidget* sw, JiControl* page);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_STACKED_WIDGET_H */
