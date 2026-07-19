/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_manager.h
 * @brief Dock manager — manages dock areas, floating windows, and drag state.
 *        Top-level coordinator for the docking system.
 */

#ifndef JIUI_DOCK_MANAGER_H
#define JIUI_DOCK_MANAGER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_dock_area.h"
#include "ji_dock_widget.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiDockDragState — tracks in-progress drag operations
 * ========================================================================= */
typedef struct JiDockDragState {
    JiDockWidget*  dragged_widget;   /* widget being dragged (NULL if none) */
    JiDockArea*    source_area;      /* area the widget was dragged from */
    JiRect         drag_rect;        /* current drag position */
    bool           is_dragging;      /* true while a drag is in progress */
    int            mouse_x;          /* current mouse X */
    int            mouse_y;          /* current mouse Y */
} JiDockDragState;

/* =========================================================================
 * JiDockManager — top-level docking system coordinator
 * ========================================================================= */
typedef struct JiDockManager {
    /* Root dock area (center region) */
    JiDockArea*  root_area;

    /* Floating dock widgets (not docked in any area) */
    JiDockWidget**  floating_widgets;
    int             floating_count;
    int             floating_capacity;

    /* All dock areas (for traversal) */
    JiDockArea**  areas;
    int           area_count;
    int           area_capacity;

    /* Drag state */
    JiDockDragState  drag_state;

    /* Active dock widget (for title bar styling) */
    JiDockWidget*  active_widget;

    /* Total geometry (the window's content area) */
    JiRect  geometry;
} JiDockManager;

/* ---- Lifecycle ---- */

/** Create a new dock manager. */
JI_API JiDockManager* ji_dock_manager_new(void);

/** Destroy a dock manager and all its areas and widgets. */
JI_API void ji_dock_manager_destroy(JiDockManager* manager);

/* ---- Area management ---- */

/** Add a dock area to the manager. */
JI_API void ji_dock_manager_add_area(JiDockManager* manager, JiDockArea* area);

/** Remove a dock area from the manager (does not destroy it). */
JI_API void ji_dock_manager_remove_area(JiDockManager* manager, JiDockArea* area);

/** Get the root dock area. */
JI_API JiDockArea* ji_dock_manager_get_root_area(const JiDockManager* manager);

/** Set the root dock area. */
JI_API void ji_dock_manager_set_root_area(JiDockManager* manager, JiDockArea* area);

/** Get the total number of dock areas. */
JI_API int ji_dock_manager_area_count(const JiDockManager* manager);

/** Get a dock area by index. */
JI_API JiDockArea* ji_dock_manager_get_area(const JiDockManager* manager, int index);

/* ---- Widget management ---- */

/** Add a dock widget to a specific area. */
JI_API void ji_dock_manager_add_widget(JiDockManager* manager,
                                          JiDockWidget* widget, JiDockArea* area);

/** Remove a dock widget from its area (makes it floating). */
JI_API void ji_dock_manager_remove_widget(JiDockManager* manager,
                                            JiDockWidget* widget);

/** Float a dock widget (undock it). */
JI_API void ji_dock_manager_float_widget(JiDockManager* manager,
                                            JiDockWidget* widget);

/** Dock a floating widget into a specific area. */
JI_API void ji_dock_manager_dock_widget(JiDockManager* manager,
                                          JiDockWidget* widget, JiDockArea* area);

/** Find a dock widget by name across all areas and floating widgets. */
JI_API JiDockWidget* ji_dock_manager_find_widget(const JiDockManager* manager,
                                                    const char* name);

/* ---- Floating widget management ---- */

/** Get the number of floating dock widgets. */
JI_API int ji_dock_manager_floating_count(const JiDockManager* manager);

/** Get a floating dock widget by index. */
JI_API JiDockWidget* ji_dock_manager_get_floating(const JiDockManager* manager, int index);

/* ---- Active widget ---- */

/** Set the active dock widget. */
JI_API void ji_dock_manager_set_active_widget(JiDockManager* manager,
                                                 JiDockWidget* widget);

/** Get the active dock widget. */
JI_API JiDockWidget* ji_dock_manager_get_active_widget(const JiDockManager* manager);

/* ---- Geometry ---- */

/** Set the total geometry of the dock manager (window content area). */
JI_API void ji_dock_manager_set_geometry(JiDockManager* manager, JiRect geometry);

/** Get the total geometry. */
JI_API JiRect ji_dock_manager_get_geometry(const JiDockManager* manager);

/* ---- Drag state ---- */

/** Begin dragging a dock widget. */
JI_API void ji_dock_manager_begin_drag(JiDockManager* manager,
                                          JiDockWidget* widget, int mouse_x, int mouse_y);

/** Update drag position. */
JI_API void ji_dock_manager_update_drag(JiDockManager* manager, int mouse_x, int mouse_y);

/** End dragging — drops the widget at the current position. */
JI_API void ji_dock_manager_end_drag(JiDockManager* manager);

/** Check if a drag is in progress. */
JI_API bool ji_dock_manager_is_dragging(const JiDockManager* manager);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_MANAGER_H */
