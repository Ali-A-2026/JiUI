/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_splitter.h
 * @brief Splitter — resizable panel with draggable handle. Supports
 *        horizontal/vertical orientation, min/max size constraints, and
 *        collapsible sections.
 */

#ifndef JIUI_SPLITTER_H
#define JIUI_SPLITTER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiSplitterOrientation
 * ========================================================================= */
typedef enum JiSplitterOrientation {
    JI_SPLITTER_HORIZONTAL = 0,
    JI_SPLITTER_VERTICAL   = 1
} JiSplitterOrientation;

/* =========================================================================
 * JiSplitterPanel — a single panel within a splitter
 * ========================================================================= */
typedef struct JiSplitterPanel {
    void*   content;        /* opaque pointer to content (JiDockArea or JiControl) */
    int     min_size;       /* minimum size in split direction (px) */
    int     max_size;       /* maximum size (0 = unlimited) */
    int     stretch_factor; /* stretch factor for proportional resizing */
    int     current_size;   /* current size in split direction (px) */
    bool    collapsible;    /* can this panel be collapsed */
    bool    is_collapsed;   /* is this panel currently collapsed */
} JiSplitterPanel;

/* =========================================================================
 * JiSplitter — resizable multi-panel container
 * ========================================================================= */
typedef struct JiSplitter {
    JiSplitterOrientation  orientation;
    JiSplitterPanel*  panels;
    int               panel_count;
    int               panel_capacity;
    JiRect            rect;
    int               handle_width;   /* default 4px */
    int               drag_index;     /* index of handle being dragged (-1 = none) */
    int               drag_start_pos; /* mouse position at drag start */
    int               drag_start_size; /* panel size at drag start */
} JiSplitter;

/* ---- Lifecycle ---- */

/** Create a new splitter. */
JI_API JiSplitter* ji_splitter_new(JiSplitterOrientation orientation);

/** Destroy a splitter (does not destroy content pointers). */
JI_API void ji_splitter_destroy(JiSplitter* splitter);

/* ---- Panel management ---- */

/** Add a panel to the splitter. */
JI_API void ji_splitter_add_panel(JiSplitter* splitter, void* content,
                                     int min_size, int stretch_factor);

/** Insert a panel at a specific index. */
JI_API void ji_splitter_insert_panel(JiSplitter* splitter, int index,
                                        void* content, int min_size, int stretch_factor);

/** Remove a panel by index (does not destroy content). */
JI_API void ji_splitter_remove_panel(JiSplitter* splitter, int index);

/** Get the number of panels. */
JI_API int ji_splitter_panel_count(const JiSplitter* splitter);

/** Get a panel by index. */
JI_API JiSplitterPanel* ji_splitter_get_panel(const JiSplitter* splitter, int index);

/* ---- Size management ---- */

/** Set explicit sizes for all panels. Array length must match panel count. */
JI_API void ji_splitter_set_sizes(JiSplitter* splitter, const int* sizes, int count);

/** Get the sizes of all panels. Caller must allocate array of panel_count ints. */
JI_API void ji_splitter_get_sizes(const JiSplitter* splitter, int* sizes, int count);

/** Set the stretch factor for a panel. */
JI_API void ji_splitter_set_stretch_factor(JiSplitter* splitter, int index, int factor);

/** Get the stretch factor for a panel. */
JI_API int ji_splitter_get_stretch_factor(const JiSplitter* splitter, int index);

/** Set min/max size constraints for a panel. */
JI_API void ji_splitter_set_size_constraints(JiSplitter* splitter, int index,
                                                int min_size, int max_size);

/* ---- Orientation ---- */

/** Set the splitter orientation. */
JI_API void ji_splitter_set_orientation(JiSplitter* splitter,
                                          JiSplitterOrientation orientation);

/** Get the splitter orientation. */
JI_API JiSplitterOrientation ji_splitter_get_orientation(const JiSplitter* splitter);

/* ---- Geometry ---- */

/** Set the splitter's rectangle. Recalculates panel sizes proportionally. */
JI_API void ji_splitter_set_rect(JiSplitter* splitter, JiRect rect);

/** Get the splitter's rectangle. */
JI_API JiRect ji_splitter_get_rect(const JiSplitter* splitter);

/** Get the rectangle for a specific panel. */
JI_API JiRect ji_splitter_get_panel_rect(const JiSplitter* splitter, int index);

/** Get the rectangle for a specific handle (between panels). */
JI_API JiRect ji_splitter_get_handle_rect(const JiSplitter* splitter, int handle_index);

/* ---- Interaction ---- */

/** Begin dragging a handle at the given mouse position. */
JI_API bool ji_splitter_begin_drag(JiSplitter* splitter, int mouse_x, int mouse_y);

/** Update drag position. Returns true if sizes changed. */
JI_API bool ji_splitter_update_drag(JiSplitter* splitter, int mouse_x, int mouse_y);

/** End dragging. */
JI_API void ji_splitter_end_drag(JiSplitter* splitter);

/** Check if a drag is in progress. */
JI_API bool ji_splitter_is_dragging(const JiSplitter* splitter);

/** Hit-test: returns the handle index at the given position, or -1 if none. */
JI_API int ji_splitter_hit_test_handle(const JiSplitter* splitter, int x, int y);

/* ---- Collapse ---- */

/** Toggle collapse state for a panel. */
JI_API void ji_splitter_toggle_collapse(JiSplitter* splitter, int index);

/** Check if a panel is collapsed. */
JI_API bool ji_splitter_is_collapsed(const JiSplitter* splitter, int index);

/** Set whether a panel is collapsible. */
JI_API void ji_splitter_set_collapsible(JiSplitter* splitter, int index, bool collapsible);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SPLITTER_H */
