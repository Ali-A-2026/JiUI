/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_widget.h
 * @brief Dock widget — a dockable panel with title bar, content, and state.
 */

#ifndef JIUI_DOCK_WIDGET_H
#define JIUI_DOCK_WIDGET_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiDockState — matches Qt6 QDockWidget::DockWidgetState
 * ========================================================================= */
typedef enum JiDockState {
    JI_DOCK_DOCKED   = 0,   /* widget is docked in a dock area */
    JI_DOCK_FLOATING = 1,   /* widget is floating in its own window */
    JI_DOCK_HIDDEN   = 2    /* widget is hidden */
} JiDockState;

/* =========================================================================
 * JiDockFeature — matches Qt6 QDockWidget::DockWidgetFeature
 * ========================================================================= */
typedef enum JiDockFeature {
    JI_DOCK_FEATURE_NONE           = 0,
    JI_DOCK_FEATURE_CLOSABLE       = 1 << 0,
    JI_DOCK_FEATURE_MOVABLE        = 1 << 1,
    JI_DOCK_FEATURE_FLOATABLE      = 1 << 2,
    JI_DOCK_FEATURE_RESIZABLE      = 1 << 3,
    JI_DOCK_FEATURE_ALL            = 0x0F
} JiDockFeature;

/* =========================================================================
 * JiDockWidget — a single dockable panel
 * ========================================================================= */
typedef struct JiDockArea JiDockArea;  /* forward decl */

typedef struct JiDockWidget {
    /* Identity */
    char*     name;             /* unique name for serialization */
    char*     title;            /* title bar text */

    /* Content */
    JiControl*  content;        /* the widget displayed inside the dock */

    /* State */
    JiDockState state;
    int         features;       /* bitmask of JiDockFeature */

    /* Parent area (NULL when floating) */
    JiDockArea* area;

    /* Title bar geometry */
    int     title_bar_height;   /* default 20px */

    /* Flags */
    bool    is_active;          /* is this the active dock widget in its area */
    bool    is_floating;        /* convenience: state == FLOATING */
} JiDockWidget;

/* ---- Lifecycle ---- */

/** Create a new dock widget with the given name and title. */
JI_API JiDockWidget* ji_dock_widget_new(const char* name, const char* title);

/** Destroy a dock widget (does not destroy the content control). */
JI_API void ji_dock_widget_destroy(JiDockWidget* widget);

/* ---- Properties ---- */

/** Set the dock widget's title text. */
JI_API void ji_dock_widget_set_title(JiDockWidget* widget, const char* title);

/** Get the dock widget's title text. */
JI_API const char* ji_dock_widget_get_title(const JiDockWidget* widget);

/** Set the content control displayed inside the dock widget. */
JI_API void ji_dock_widget_set_widget(JiDockWidget* widget, JiControl* content);

/** Get the content control. */
JI_API JiControl* ji_dock_widget_get_widget(const JiDockWidget* widget);

/** Set the dock widget features (closable, movable, floatable, etc.). */
JI_API void ji_dock_widget_set_features(JiDockWidget* widget, int features);

/** Get the dock widget features. */
JI_API int ji_dock_widget_get_features(const JiDockWidget* widget);

/** Set the dock widget's name (for serialization). */
JI_API void ji_dock_widget_set_name(JiDockWidget* widget, const char* name);

/** Get the dock widget's name. */
JI_API const char* ji_dock_widget_get_name(const JiDockWidget* widget);

/* ---- State ---- */

/** Make the dock widget floating (undock it). */
JI_API void ji_dock_widget_set_floating(JiDockWidget* widget, bool floating);

/** Check if the dock widget is currently floating. */
JI_API bool ji_dock_widget_is_floating(const JiDockWidget* widget);

/** Hide the dock widget. */
JI_API void ji_dock_widget_hide(JiDockWidget* widget);

/** Show the dock widget (if previously hidden). */
JI_API void ji_dock_widget_show(JiDockWidget* widget);

/** Check if the dock widget is hidden. */
JI_API bool ji_dock_widget_is_hidden(const JiDockWidget* widget);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_WIDGET_H */
