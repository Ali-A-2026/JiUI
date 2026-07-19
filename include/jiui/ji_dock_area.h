/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_area.h
 * @brief Dock area — a region containing dock widgets, with tab bar and
 *        splitter support for resizable sub-areas.
 */

#ifndef JIUI_DOCK_AREA_H
#define JIUI_DOCK_AREA_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_dock_widget.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiDockRegion — matches Qt6 dock area positions
 * ========================================================================= */
typedef enum JiDockRegion {
    JI_DOCK_REGION_LEFT   = 0,
    JI_DOCK_REGION_RIGHT  = 1,
    JI_DOCK_REGION_TOP    = 2,
    JI_DOCK_REGION_BOTTOM = 3,
    JI_DOCK_REGION_CENTER = 4
} JiDockRegion;

/* =========================================================================
 * JiDockArea — a container for dock widgets
 * ========================================================================= */
typedef struct JiSplitter JiSplitter;  /* forward decl */

typedef struct JiDockArea {
    /* Identity */
    char*     name;

    /* Region within parent (if part of a splitter) */
    JiDockRegion  region;

    /* Dock widgets in this area (tabified if > 1) */
    JiDockWidget**  widgets;
    int             widget_count;
    int             widget_capacity;

    /* Active tab index (which widget is visible) */
    int     active_index;

    /* Splitter children (for nested resizable areas) */
    JiSplitter*  splitter;     /* NULL if no splitter children */

    /* Geometry */
    JiRect  rect;

    /* Tab bar height */
    int     tab_bar_height;    /* default 24px */
} JiDockArea;

/* ---- Lifecycle ---- */

/** Create a new dock area. */
JI_API JiDockArea* ji_dock_area_new(const char* name);

/** Destroy a dock area and all contained dock widgets. */
JI_API void ji_dock_area_destroy(JiDockArea* area);

/* ---- Widget management ---- */

/** Add a dock widget to this area. */
JI_API void ji_dock_area_add_widget(JiDockArea* area, JiDockWidget* widget);

/** Remove a dock widget from this area (does not destroy it). */
JI_API void ji_dock_area_remove_widget(JiDockArea* area, JiDockWidget* widget);

/** Get the number of dock widgets in this area. */
JI_API int ji_dock_area_widget_count(const JiDockArea* area);

/** Get a dock widget by index. */
JI_API JiDockWidget* ji_dock_area_get_widget(const JiDockArea* area, int index);

/** Find a dock widget by name. */
JI_API JiDockWidget* ji_dock_area_find_widget(const JiDockArea* area, const char* name);

/* ---- Tab management ---- */

/** Set the active (visible) dock widget by index. */
JI_API void ji_dock_area_set_active_index(JiDockArea* area, int index);

/** Get the active (visible) dock widget index. */
JI_API int ji_dock_area_get_active_index(const JiDockArea* area);

/** Get the active dock widget. */
JI_API JiDockWidget* ji_dock_area_get_active_widget(const JiDockArea* area);

/* ---- Geometry ---- */

/** Set the dock area's rectangle. */
JI_API void ji_dock_area_set_rect(JiDockArea* area, JiRect rect);

/** Get the dock area's rectangle. */
JI_API JiRect ji_dock_area_get_rect(const JiDockArea* area);

/* ---- Properties ---- */

/** Set the dock area's name. */
JI_API void ji_dock_area_set_name(JiDockArea* area, const char* name);

/** Get the dock area's name. */
JI_API const char* ji_dock_area_get_name(const JiDockArea* area);

/** Set the region of this dock area. */
JI_API void ji_dock_area_set_region(JiDockArea* area, JiDockRegion region);

/** Get the region of this dock area. */
JI_API JiDockRegion ji_dock_area_get_region(const JiDockArea* area);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_AREA_H */
