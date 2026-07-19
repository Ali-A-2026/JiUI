/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_layout.h
 * @brief Layout system — measure/arrange passes, layout element, alignment.
 */

#ifndef JIUI_LAYOUT_H
#define JIUI_LAYOUT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include "ji_object.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Alignment enums
 * ========================================================================= */
typedef enum JiHorizontalAlignment {
    JI_ALIGN_H_LEFT    = 0,
    JI_ALIGN_H_CENTER  = 1,
    JI_ALIGN_H_RIGHT   = 2,
    JI_ALIGN_H_STRETCH  = 3
} JiHorizontalAlignment;

typedef enum JiVerticalAlignment {
    JI_ALIGN_V_TOP     = 0,
    JI_ALIGN_V_CENTER   = 1,
    JI_ALIGN_V_BOTTOM   = 2,
    JI_ALIGN_V_STRETCH   = 3
} JiVerticalAlignment;

typedef enum JiOrientation {
    JI_ORIENTATION_HORIZONTAL = 0,
    JI_ORIENTATION_VERTICAL   = 1
} JiOrientation;

typedef enum JiDock {
    JI_DOCK_LEFT   = 0,
    JI_DOCK_TOP    = 1,
    JI_DOCK_RIGHT  = 2,
    JI_DOCK_BOTTOM = 3
} JiDock;

/* =========================================================================
 * Layout state flags
 * ========================================================================= */
typedef enum JiLayoutFlags {
    JI_LAYOUT_MEASURE_DIRTY    = 1 << 0,
    JI_LAYOUT_ARRANGE_DIRTY    = 1 << 1,
    JI_LAYOUT_IS_MEASURING     = 1 << 2,
    JI_LAYOUT_IS_ARRANGING     = 1 << 3,
    JI_LAYOUT_IS_LAYOUT_CYCLE  = 1 << 4
} JiLayoutFlags;

/* =========================================================================
 * JiLayoutElement — extends JiObject with layout properties
 *
 * Layout follows the WPF-style two-pass model:
 *   1. Measure:  each element computes its desired size given available space
 *   2. Arrange:  parent positions and sizes each child within its final rect
 * ========================================================================= */
typedef struct JiLayoutElement JiLayoutElement;

struct JiLayoutElement {
    JiObject  base;                     /* must be first for casting */

    /* Layout properties */
    JiHorizontalAlignment horizontal_alignment;
    JiVerticalAlignment   vertical_alignment;
    JiThickness           margin;
    JiThickness           padding;
    double                width;         /* explicit width (NaN = auto) */
    double                height;        /* explicit height (NaN = auto) */
    double                min_width;
    double                max_width;
    double                min_height;
    double                max_height;

    /* Computed layout state */
    JiSize    desired_size;              /* result of measure pass */
    JiRect    layout_slot;               /* final arranged rect */
    JiSize    previous_available_size;   /* last available size passed to measure */
    int       layout_flags;             /* JiLayoutFlags bitmask */

    /* Parent back-reference */
    JiLayoutElement* layout_parent;

    /* Virtual table for layout */
    JiSize  (*measure_override)(JiLayoutElement* self, JiSize available_size);
    JiRect  (*arrange_override)(JiLayoutElement* self, JiRect final_rect);
};

/* =========================================================================
 * NaN helper for auto-sizing
 * ========================================================================= */
JI_API bool ji_is_nan(double val);
JI_API double ji_nan(void);

/* =========================================================================
 * Layout Element API
 * ========================================================================= */

/** Create a new layout element. */
JI_API JiLayoutElement* ji_layout_element_new(JiTypeId type_id);

/** Destroy a layout element. */
JI_API void ji_layout_element_destroy(JiLayoutElement* elem);

/** Measure: compute desired size given available space. */
JI_API void ji_layout_element_measure(JiLayoutElement* elem, JiSize available_size);

/** Arrange: position and size the element within the final rect. */
JI_API void ji_layout_element_arrange(JiLayoutElement* elem, JiRect final_rect);

/** Invalidate the measure state (mark dirty). */
JI_API void ji_layout_element_invalidate_measure(JiLayoutElement* elem);

/** Invalidate the arrange state (mark dirty). */
JI_API void ji_layout_element_invalidate_arrange(JiLayoutElement* elem);

/** Force a layout update (measure + arrange if dirty). */
JI_API void ji_layout_element_update_layout(JiLayoutElement* elem);

/** Get the desired size (valid after measure). */
JI_API JiSize ji_layout_element_get_desired_size(const JiLayoutElement* elem);

/** Get the render size (valid after arrange). */
JI_API JiSize ji_layout_element_get_render_size(const JiLayoutElement* elem);

/** Get the layout slot (valid after arrange). */
JI_API JiRect ji_layout_element_get_layout_slot(const JiLayoutElement* elem);

/** Check if measure is dirty. */
JI_API bool ji_layout_element_is_measure_dirty(const JiLayoutElement* elem);

/** Check if arrange is dirty. */
JI_API bool ji_layout_element_is_arrange_dirty(const JiLayoutElement* elem);

/* =========================================================================
 * Property helpers — clamp size to min/max, apply margin/padding
 * ========================================================================= */

/** Clamp a size to the element's min/max constraints. */
JI_API JiSize ji_layout_element_clamp_size(const JiLayoutElement* elem, JiSize size);

/** Collapse margin: subtract margin from available space. */
JI_API JiSize ji_layout_element_collapse_margin(const JiLayoutElement* elem, JiSize available);

/** Expand margin: add margin to the element's size. */
JI_API JiSize ji_layout_element_expand_margin(const JiLayoutElement* elem, JiSize size);

/** Collapse padding: subtract padding from available space. */
JI_API JiSize ji_layout_element_collapse_padding(const JiLayoutElement* elem, JiSize available);

/** Expand padding: add padding to the element's size. */
JI_API JiSize ji_layout_element_expand_padding(const JiLayoutElement* elem, JiSize size);

/* =========================================================================
 * Default measure/arrange overrides
 * ========================================================================= */

/** Default measure override: returns the element's explicit size or 0. */
JI_API JiSize ji_layout_element_measure_override(JiLayoutElement* self, JiSize available_size);

/** Default arrange override: returns the final rect as-is. */
JI_API JiRect ji_layout_element_arrange_override(JiLayoutElement* self, JiRect final_rect);

/* =========================================================================
 * Layout type registration
 * ========================================================================= */

/** Get the JiTypeId for JiLayoutElement. */
JI_API JiTypeId ji_layout_element_type_id(void);

/** Initialize the layout type system (called by ji_initialize). */
JI_API void ji_layout_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_LAYOUT_H */
