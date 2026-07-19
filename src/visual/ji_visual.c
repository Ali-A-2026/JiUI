/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_visual.c
 * @brief Implementation of the visual base class.
 */

#include "jiui/ji_visual.h"
#include "jiui/ji_drawing.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_type.h"
#include "jiui/ji_object.h"

#include <string.h>
#include <math.h>

/* =========================================================================
 * Static type ID
 * ========================================================================= */
static JiTypeId s_visual_type = JI_TYPE_INVALID;

JiTypeId ji_visual_type_id(void) { return s_visual_type; }

/* =========================================================================
 * Lifetime
 * ========================================================================= */
JiVisual* ji_visual_new(void) {
    JiVisual* v = JI_NEW(JiVisual);
    if (!v) return NULL;
    memset(v, 0, sizeof(JiVisual));

    /* Initialize layout element (first field) */
    JiLayoutElement* layout = &v->layout;
    ji_object_init(&layout->base, s_visual_type);
    layout->horizontal_alignment = JI_ALIGN_H_STRETCH;
    layout->vertical_alignment   = JI_ALIGN_V_STRETCH;
    layout->margin  = ji_thickness_zero();
    layout->padding = ji_thickness_zero();
    layout->width   = ji_nan();
    layout->height  = ji_nan();
    layout->min_width  = 0.0;
    layout->max_width  = 1e9;
    layout->min_height = 0.0;
    layout->max_height = 1e9;
    layout->desired_size = ji_size(0, 0);
    layout->layout_slot  = ji_rect(0, 0, 0, 0);
    layout->previous_available_size = ji_size(0, 0);
    layout->layout_flags = JI_LAYOUT_MEASURE_DIRTY | JI_LAYOUT_ARRANGE_DIRTY;
    layout->layout_parent = NULL;

    /* Visual-specific defaults */
    v->visual_parent   = NULL;
    v->visual_children = NULL;
    v->visual_child_count = 0;
    v->visual_child_capacity = 0;

    v->opacity         = 1.0;
    v->visibility      = JI_VISIBILITY_VISIBLE;
    v->clip_rect       = ji_rect(0, 0, 0, 0);
    v->render_transform = ji_matrix_identity();
    v->z_index         = 0;
    v->is_visible      = true;
    v->is_dirty        = true;
    v->on_render       = NULL;

    return v;
}

void ji_visual_destroy(JiVisual* visual) {
    if (!visual) return;
    for (int i = 0; i < visual->visual_child_count; i++) {
        ji_visual_destroy(visual->visual_children[i]);
    }
    ji_free(visual->visual_children);
    ji_object_destroy(&visual->layout.base);
}

/* =========================================================================
 * Visual tree
 * ========================================================================= */
void ji_visual_add_child(JiVisual* parent, JiVisual* child) {
    if (!parent || !child) return;

    /* Reparent: remove from old parent */
    if (child->visual_parent) {
        ji_visual_remove_child(child->visual_parent, child);
    }

    /* Grow array if needed */
    if (parent->visual_child_count >= parent->visual_child_capacity) {
        int new_cap = parent->visual_child_capacity == 0 ? 8 : parent->visual_child_capacity * 2;
        parent->visual_children = (JiVisual**)ji_realloc(parent->visual_children,
            (size_t)new_cap * sizeof(JiVisual*));
        parent->visual_child_capacity = new_cap;
    }

    parent->visual_children[parent->visual_child_count++] = child;
    child->visual_parent = parent;

    /* Update effective visibility */
    child->is_visible = parent->is_visible &&
        (child->visibility == JI_VISIBILITY_VISIBLE);

    ji_visual_invalidate(parent);
}

bool ji_visual_remove_child(JiVisual* parent, JiVisual* child) {
    if (!parent || !child) return false;
    for (int i = 0; i < parent->visual_child_count; i++) {
        if (parent->visual_children[i] == child) {
            parent->visual_children[i] = parent->visual_children[parent->visual_child_count - 1];
            parent->visual_child_count--;
            child->visual_parent = NULL;
            ji_visual_invalidate(parent);
            return true;
        }
    }
    return false;
}

int ji_visual_get_child_count(const JiVisual* visual) {
    return visual ? visual->visual_child_count : 0;
}

JiVisual* ji_visual_get_child(const JiVisual* visual, int index) {
    if (!visual || index < 0 || index >= visual->visual_child_count) return NULL;
    return visual->visual_children[index];
}

/* =========================================================================
 * Properties
 * ========================================================================= */
void ji_visual_set_opacity(JiVisual* visual, double opacity) {
    if (!visual) return;
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;
    visual->opacity = opacity;
    ji_visual_invalidate(visual);
}

double ji_visual_get_opacity(const JiVisual* visual) {
    return visual ? visual->opacity : 0.0;
}

void ji_visual_set_visibility(JiVisual* visual, JiVisibility vis) {
    if (!visual) return;
    visual->visibility = vis;

    /* Update effective visibility for self and descendants */
    bool parent_visible = true;
    if (visual->visual_parent) {
        parent_visible = visual->visual_parent->is_visible;
    }
    visual->is_visible = parent_visible && (vis == JI_VISIBILITY_VISIBLE);

    /* Propagate to children */
    for (int i = 0; i < visual->visual_child_count; i++) {
        JiVisual* child = visual->visual_children[i];
        child->is_visible = visual->is_visible &&
            (child->visibility == JI_VISIBILITY_VISIBLE);
    }

    ji_visual_invalidate(visual);
}

JiVisibility ji_visual_get_visibility(const JiVisual* visual) {
    return visual ? visual->visibility : JI_VISIBILITY_COLLAPSED;
}

void ji_visual_set_clip(JiVisual* visual, JiRect clip) {
    if (!visual) return;
    visual->clip_rect = clip;
    ji_visual_invalidate(visual);
}

JiRect ji_visual_get_clip(const JiVisual* visual) {
    return visual ? visual->clip_rect : ji_rect(0, 0, 0, 0);
}

void ji_visual_set_render_transform(JiVisual* visual, JiMatrix transform) {
    if (!visual) return;
    visual->render_transform = transform;
    ji_visual_invalidate(visual);
}

JiMatrix ji_visual_get_render_transform(const JiVisual* visual) {
    return visual ? visual->render_transform : ji_matrix_identity();
}

void ji_visual_set_z_index(JiVisual* visual, int z_index) {
    if (!visual) return;
    visual->z_index = z_index;
    ji_visual_invalidate(visual);
}

int ji_visual_get_z_index(const JiVisual* visual) {
    return visual ? visual->z_index : 0;
}

bool ji_visual_is_effectively_visible(const JiVisual* visual) {
    if (!visual) return false;
    /* Walk up the visual tree */
    const JiVisual* v = visual;
    while (v) {
        if (v->visibility != JI_VISIBILITY_VISIBLE) return false;
        v = v->visual_parent;
    }
    return true;
}

void ji_visual_invalidate(JiVisual* visual) {
    if (!visual) return;
    visual->is_dirty = true;
}

/* =========================================================================
 * Rendering
 * ========================================================================= */
void ji_visual_render(JiVisual* visual, JiDrawingContext* ctx) {
    if (!visual || !visual->is_visible) return;

    /* Push transform if not identity */
    bool has_transform = !ji_matrix_is_identity(visual->render_transform);
    if (has_transform) {
        ji_draw_push_transform(ctx, visual->render_transform);
    }

    /* Push clip if set (non-zero size) */
    bool has_clip = (visual->clip_rect.width > 0 && visual->clip_rect.height > 0);
    if (has_clip) {
        ji_draw_push_clip(ctx, visual->clip_rect);
    }

    /* Apply opacity — in a real renderer this would push an opacity group */
    /* For now we just call the render callback */

    /* Call the render callback */
    if (visual->on_render) {
        visual->on_render(visual, ctx);
    }

    /* Render children (sorted by z_index in a real implementation) */
    for (int i = 0; i < visual->visual_child_count; i++) {
        ji_visual_render(visual->visual_children[i], ctx);
    }

    /* Pop clip */
    if (has_clip) {
        ji_draw_pop_clip(ctx);
    }

    /* Pop transform */
    if (has_transform) {
        ji_draw_pop_transform(ctx);
    }

    visual->is_dirty = false;
}

/* =========================================================================
 * Type registration
 * ========================================================================= */
void ji_visual_type_init(void) {
    s_visual_type = ji_type_register("JiVisual", sizeof(JiVisual),
                                     ji_layout_element_type_id(), NULL, NULL);
}
