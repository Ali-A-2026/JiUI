/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_visual.h
 * @brief Visual base class — render transform, opacity, clip, visibility,
 *        visual tree parent/child, and rendering hooks.
 *
 * Core visual interface. Every element that can be
 * rendered on screen derives from JiVisual.
 */

#ifndef JIUI_VISUAL_H
#define JIUI_VISUAL_H

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
 * Visibility enum
 * ========================================================================= */
typedef enum JiVisibility {
    JI_VISIBILITY_VISIBLE   = 0,  /* element is visible and participates in layout */
    JI_VISIBILITY_HIDDEN    = 1,  /* element is invisible but still occupies space */
    JI_VISIBILITY_COLLAPSED = 2   /* element is invisible and does not occupy space */
} JiVisibility;

/* =========================================================================
 * JiVisual — base visual element
 * ========================================================================= */
typedef struct JiDrawingContext JiDrawingContext;

typedef struct JiVisual {
    JiLayoutElement layout;          /* must be first */

    /* Visual tree */
    struct JiVisual* visual_parent;
    struct JiVisual** visual_children;
    int               visual_child_count;
    int               visual_child_capacity;

    /* Rendering properties */
    double       opacity;            /* 0.0 .. 1.0 */
    JiVisibility visibility;
    JiRect       clip_rect;         /* optional clip; zero-size means no clip */
    JiMatrix    render_transform;   /* 2D affine transform */
    int          z_index;           /* sibling Z order */

    /* Computed state */
    bool is_visible;                /* effective visibility (parent chain) */
    bool is_dirty;                   /* needs re-render */

    /** Render callback — override to draw content. */
    void (*on_render)(struct JiVisual* self, JiDrawingContext* ctx);
} JiVisual;

/* ---- Lifetime ---- */

/** Create a new visual element. */
JI_API JiVisual* ji_visual_new(void);

/** Destroy a visual element and all its visual children. */
JI_API void ji_visual_destroy(JiVisual* visual);

/* ---- Visual tree ---- */

/** Add a visual child. Reparents if already attached. */
JI_API void ji_visual_add_child(JiVisual* parent, JiVisual* child);

/** Remove a visual child (does not destroy it). */
JI_API bool ji_visual_remove_child(JiVisual* parent, JiVisual* child);

/** Get the number of visual children. */
JI_API int ji_visual_get_child_count(const JiVisual* visual);

/** Get a visual child by index. */
JI_API JiVisual* ji_visual_get_child(const JiVisual* visual, int index);

/* ---- Properties ---- */

/** Set the opacity (clamped to 0..1). */
JI_API void ji_visual_set_opacity(JiVisual* visual, double opacity);

/** Get the opacity. */
JI_API double ji_visual_get_opacity(const JiVisual* visual);

/** Set the visibility. */
JI_API void ji_visual_set_visibility(JiVisual* visual, JiVisibility vis);

/** Get the visibility. */
JI_API JiVisibility ji_visual_get_visibility(const JiVisual* visual);

/** Set the clip rectangle (zero-size means no clip). */
JI_API void ji_visual_set_clip(JiVisual* visual, JiRect clip);

/** Get the clip rectangle. */
JI_API JiRect ji_visual_get_clip(const JiVisual* visual);

/** Set the render transform. */
JI_API void ji_visual_set_render_transform(JiVisual* visual, JiMatrix transform);

/** Get the render transform. */
JI_API JiMatrix ji_visual_get_render_transform(const JiVisual* visual);

/** Set the Z-index. */
JI_API void ji_visual_set_z_index(JiVisual* visual, int z_index);

/** Get the Z-index. */
JI_API int ji_visual_get_z_index(const JiVisual* visual);

/** Check if the visual is effectively visible (all ancestors visible). */
JI_API bool ji_visual_is_effectively_visible(const JiVisual* visual);

/** Invalidate the visual (mark as needing re-render). */
JI_API void ji_visual_invalidate(JiVisual* visual);

/* ---- Rendering ---- */

/** Render the visual and its children using the given drawing context. */
JI_API void ji_visual_render(JiVisual* visual, JiDrawingContext* ctx);

/** Get the JiTypeId for JiVisual. */
JI_API JiTypeId ji_visual_type_id(void);

/** Initialize the visual type (called during ji_initialize). */
JI_API void ji_visual_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_VISUAL_H */
