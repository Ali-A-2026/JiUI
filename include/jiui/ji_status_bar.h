/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_status_bar.h
 * @brief StatusBar — bottom bar with message slots and permanent widgets.
 */

#ifndef JIUI_STATUS_BAR_H
#define JIUI_STATUS_BAR_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiStatusBarItem {
    char*      label;
    JiControl* widget;
    bool       is_permanent;  /* permanent items stay on the right */
    int        stretch;
} JiStatusBarItem;

typedef struct JiStatusBar {
    JiControl         control;        /* must be first */
    JiStatusBarItem*  items;
    int               item_count;
    int               item_capacity;
    char*             current_message;
    int               height;         /* default 22px */
} JiStatusBar;

/** Create a new status bar. */
JI_API JiStatusBar* ji_status_bar_new(void);

/** Destroy a status bar. */
JI_API void ji_status_bar_destroy(JiStatusBar* sb);

/** Show a temporary message. */
JI_API void ji_status_bar_show_message(JiStatusBar* sb, const char* message);

/** Clear the temporary message. */
JI_API void ji_status_bar_clear_message(JiStatusBar* sb);

/** Add a permanent widget (right side). */
JI_API void ji_status_bar_add_permanent_widget(JiStatusBar* sb, JiControl* widget);

/** Add a temporary widget (left side). */
JI_API void ji_status_bar_add_widget(JiStatusBar* sb, JiControl* widget);

/** Get the current message. */
JI_API const char* ji_status_bar_get_current_message(const JiStatusBar* sb);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_STATUS_BAR_H */
