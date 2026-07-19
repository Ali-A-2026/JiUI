/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_panel.h
 * @brief Panel types — StackPanel, Grid, Canvas, WrapPanel, DockPanel.
 */

#ifndef JIUI_PANEL_H
#define JIUI_PANEL_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_layout.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiPanel — base panel with children collection
 * ========================================================================= */
typedef struct JiPanel {
    JiLayoutElement layout;      /* must be first */

    /* Children array */
    JiLayoutElement** children;
    int               child_count;
    int               child_capacity;

    /* Background color (ARGB) */
    unsigned long background;
} JiPanel;

/** Create a new panel. */
JI_API JiPanel* ji_panel_new(JiTypeId type_id);

/** Destroy a panel and all its children. */
JI_API void ji_panel_destroy(JiPanel* panel);

/** Add a child to the panel. */
JI_API void ji_panel_add_child(JiPanel* panel, JiLayoutElement* child);

/** Remove a child from the panel (does not destroy it). */
JI_API bool ji_panel_remove_child(JiPanel* panel, JiLayoutElement* child);

/** Get the number of children. */
JI_API int ji_panel_get_child_count(const JiPanel* panel);

/** Get a child by index. */
JI_API JiLayoutElement* ji_panel_get_child(const JiPanel* panel, int index);

/** Get the JiTypeId for JiPanel. */
JI_API JiTypeId ji_panel_type_id(void);

/* =========================================================================
 * JiStackPanel — stacks children horizontally or vertically
 * ========================================================================= */
typedef struct JiStackPanel {
    JiPanel panel;                /* must be first */

    JiOrientation orientation;
    double        spacing;        /* gap between children */
} JiStackPanel;

/** Create a new StackPanel. */
JI_API JiStackPanel* ji_stack_panel_new(void);

/** Destroy a StackPanel. */
JI_API void ji_stack_panel_destroy(JiStackPanel* sp);

/** Set the orientation. */
JI_API void ji_stack_panel_set_orientation(JiStackPanel* sp, JiOrientation orientation);

/** Set the spacing between children. */
JI_API void ji_stack_panel_set_spacing(JiStackPanel* sp, double spacing);

/** Get the JiTypeId for JiStackPanel. */
JI_API JiTypeId ji_stack_panel_type_id(void);

/* =========================================================================
 * JiGrid — grid layout with rows and columns
 * ========================================================================= */
typedef struct JiGridRowColDef {
    JiGridLength length;    /* auto, pixel, or star */
    double       actual;    /* computed actual size after measure */
    double       offset;    /* computed offset from start */
} JiGridRowColDef;

typedef struct JiGrid {
    JiPanel panel;                /* must be first */

    /* Row and column definitions */
    JiGridRowColDef* rows;
    int               row_count;
    int               row_capacity;

    JiGridRowColDef* cols;
    int               col_count;
    int               col_capacity;

    /* Spacing */
    double row_spacing;
    double col_spacing;

    /* Show grid lines (debug) */
    bool show_grid_lines;
} JiGrid;

/** Create a new Grid. */
JI_API JiGrid* ji_grid_new(void);

/** Destroy a Grid. */
JI_API void ji_grid_destroy(JiGrid* grid);

/** Add a row definition. */
JI_API void ji_grid_add_row(JiGrid* grid, JiGridLength length);

/** Add a column definition. */
JI_API void ji_grid_add_column(JiGrid* grid, JiGridLength length);

/** Set row spacing. */
JI_API void ji_grid_set_row_spacing(JiGrid* grid, double spacing);

/** Set column spacing. */
JI_API void ji_grid_set_col_spacing(JiGrid* grid, double spacing);

/** Get the number of rows. */
JI_API int ji_grid_get_row_count(const JiGrid* grid);

/** Get the number of columns. */
JI_API int ji_grid_get_col_count(const JiGrid* grid);

/** Get the JiTypeId for JiGrid. */
JI_API JiTypeId ji_grid_type_id(void);

/* =========================================================================
 * JiCanvas — absolute positioning panel
 * ========================================================================= */
typedef struct JiCanvas {
    JiPanel panel;                /* must be first */
} JiCanvas;

/** Create a new Canvas. */
JI_API JiCanvas* ji_canvas_new(void);

/** Destroy a Canvas. */
JI_API void ji_canvas_destroy(JiCanvas* canvas);

/** Set the Left attached property for a child. */
JI_API void ji_canvas_set_left(JiLayoutElement* child, double left);

/** Set the Top attached property for a child. */
JI_API void ji_canvas_set_top(JiLayoutElement* child, double top);

/** Get the Left attached property for a child. */
JI_API double ji_canvas_get_left(const JiLayoutElement* child);

/** Get the Top attached property for a child. */
JI_API double ji_canvas_get_top(const JiLayoutElement* child);

/** Get the JiTypeId for JiCanvas. */
JI_API JiTypeId ji_canvas_type_id(void);

/* =========================================================================
 * JiWrapPanel — wraps children to next line when space runs out
 * ========================================================================= */
typedef struct JiWrapPanel {
    JiPanel panel;                /* must be first */

    JiOrientation orientation;
    double        item_width;     /* uniform item width (0 = auto) */
    double        item_height;    /* uniform item height (0 = auto) */
} JiWrapPanel;

/** Create a new WrapPanel. */
JI_API JiWrapPanel* ji_wrap_panel_new(void);

/** Destroy a WrapPanel. */
JI_API void ji_wrap_panel_destroy(JiWrapPanel* wp);

/** Set the orientation. */
JI_API void ji_wrap_panel_set_orientation(JiWrapPanel* wp, JiOrientation orientation);

/** Set uniform item width (0 = auto). */
JI_API void ji_wrap_panel_set_item_width(JiWrapPanel* wp, double width);

/** Set uniform item height (0 = auto). */
JI_API void ji_wrap_panel_set_item_height(JiWrapPanel* wp, double height);

/** Get the JiTypeId for JiWrapPanel. */
JI_API JiTypeId ji_wrap_panel_type_id(void);

/* =========================================================================
 * JiDockPanel — docks children to edges
 * ========================================================================= */
typedef struct JiDockPanel {
    JiPanel panel;                /* must be first */

    bool last_child_fill;         /* whether the last child fills remaining space */
} JiDockPanel;

/** Create a new DockPanel. */
JI_API JiDockPanel* ji_dock_panel_new(void);

/** Destroy a DockPanel. */
JI_API void ji_dock_panel_destroy(JiDockPanel* dp);

/** Set whether the last child fills remaining space. */
JI_API void ji_dock_panel_set_last_child_fill(JiDockPanel* dp, bool fill);

/** Set the Dock attached property for a child. */
JI_API void ji_dock_panel_set_dock(JiLayoutElement* child, JiDock dock);

/** Get the Dock attached property for a child. */
JI_API JiDock ji_dock_panel_get_dock(const JiLayoutElement* child);

/** Get the JiTypeId for JiDockPanel. */
JI_API JiTypeId ji_dock_panel_type_id(void);

/* =========================================================================
 * Panel type registration
 * ========================================================================= */

/** Initialize all panel types (called by ji_initialize). */
JI_API void ji_panel_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PANEL_H */
