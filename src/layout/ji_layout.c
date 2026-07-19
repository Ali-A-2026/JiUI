/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_layout.c
 * @brief Implementation of the layout system — measure/arrange passes.
 */

#include "jiui/ji_layout.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * NaN helpers
 * ========================================================================= */
static const union { unsigned long long u; double d; } ji_nan_bits = { 0x7FF8000000000000ULL };

double ji_nan(void) {
    return ji_nan_bits.d;
}

bool ji_is_nan(double val) {
    return val != val; /* IEEE 754 NaN is not equal to itself */
}

/* =========================================================================
 * Static type ID storage
 * ========================================================================= */
static JiTypeId s_layout_element_type = JI_TYPE_INVALID;

JiTypeId ji_layout_element_type_id(void) {
    return s_layout_element_type;
}

/* =========================================================================
 * Layout Element construction / destruction
 * ========================================================================= */
JiLayoutElement* ji_layout_element_new(JiTypeId type_id) {
    JiLayoutElement* elem = JI_NEW(JiLayoutElement);
    if (!elem) return NULL;

    memset(elem, 0, sizeof(JiLayoutElement));

    /* Initialize the JiObject base */
    ji_object_init(&elem->base, type_id);

    /* Default layout properties */
    elem->horizontal_alignment = JI_ALIGN_H_STRETCH;
    elem->vertical_alignment   = JI_ALIGN_V_STRETCH;
    elem->margin  = ji_thickness_zero();
    elem->padding = ji_thickness_zero();
    elem->width   = ji_nan();   /* auto */
    elem->height  = ji_nan();   /* auto */
    elem->min_width  = 0.0;
    elem->max_width  = 1e9;
    elem->min_height = 0.0;
    elem->max_height = 1e9;

    /* Layout state */
    elem->desired_size = ji_size(0, 0);
    elem->layout_slot  = ji_rect(0, 0, 0, 0);
    elem->previous_available_size = ji_size(0, 0);
    elem->layout_flags = JI_LAYOUT_MEASURE_DIRTY | JI_LAYOUT_ARRANGE_DIRTY;
    elem->layout_parent = NULL;

    /* Virtual table — defaults */
    elem->measure_override = ji_layout_element_measure_override;
    elem->arrange_override = ji_layout_element_arrange_override;

    return elem;
}

void ji_layout_element_destroy(JiLayoutElement* elem) {
    if (!elem) return;
    ji_object_destroy(&elem->base);
}

/* =========================================================================
 * Default measure/arrange overrides
 * ========================================================================= */
JiSize ji_layout_element_measure_override(JiLayoutElement* self, JiSize available_size) {
    (void)available_size; /* unused in default */
    double w = ji_is_nan(self->width)  ? 0.0 : self->width;
    double h = ji_is_nan(self->height) ? 0.0 : self->height;
    return ji_size(w, h);
}

JiRect ji_layout_element_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    return final_rect;
}

/* =========================================================================
 * Measure / Arrange
 * ========================================================================= */
void ji_layout_element_measure(JiLayoutElement* elem, JiSize available_size) {
    if (!elem) return;

    /* Prevent re-entrancy */
    if (elem->layout_flags & JI_LAYOUT_IS_MEASURING) return;
    elem->layout_flags |= JI_LAYOUT_IS_MEASURING;

    /* Collapse margin from available space */
    JiSize margin_available = ji_layout_element_collapse_margin(elem, available_size);

    /* Clamp to min/max */
    margin_available = ji_layout_element_clamp_size(elem, margin_available);

    /* Call the virtual measure override */
    JiSize desired;
    if (elem->measure_override) {
        desired = elem->measure_override(elem, margin_available);
    } else {
        desired = ji_size(0, 0);
    }

    /* Expand desired size by margin */
    desired = ji_layout_element_expand_margin(elem, desired);

    /* Clamp desired size */
    if (desired.width < 0.0) desired.width = 0.0;
    if (desired.height < 0.0) desired.height = 0.0;

    elem->desired_size = desired;
    elem->previous_available_size = available_size;
    elem->layout_flags &= ~JI_LAYOUT_MEASURE_DIRTY;
    elem->layout_flags &= ~JI_LAYOUT_IS_MEASURING;
}

void ji_layout_element_arrange(JiLayoutElement* elem, JiRect final_rect) {
    if (!elem) return;

    /* Prevent re-entrancy */
    if (elem->layout_flags & JI_LAYOUT_IS_ARRANGING) return;
    elem->layout_flags |= JI_LAYOUT_IS_ARRANGING;

    /* Apply alignment within the final rect */
    JiRect arranged = final_rect;

    if (elem->horizontal_alignment != JI_ALIGN_H_STRETCH ||
        elem->vertical_alignment != JI_ALIGN_V_STRETCH) {
        double desired_w = ji_is_nan(elem->width) ? elem->desired_size.width : elem->width;
        double desired_h = ji_is_nan(elem->height) ? elem->desired_size.height : elem->height;

        /* Horizontal alignment */
        switch (elem->horizontal_alignment) {
            case JI_ALIGN_H_LEFT:
                arranged.width = desired_w;
                break;
            case JI_ALIGN_H_CENTER:
                arranged.x += (arranged.width - desired_w) * 0.5;
                arranged.width = desired_w;
                break;
            case JI_ALIGN_H_RIGHT:
                arranged.x += arranged.width - desired_w;
                arranged.width = desired_w;
                break;
            case JI_ALIGN_H_STRETCH:
            default:
                break;
        }

        /* Vertical alignment */
        switch (elem->vertical_alignment) {
            case JI_ALIGN_V_TOP:
                arranged.height = desired_h;
                break;
            case JI_ALIGN_V_CENTER:
                arranged.y += (arranged.height - desired_h) * 0.5;
                arranged.height = desired_h;
                break;
            case JI_ALIGN_V_BOTTOM:
                arranged.y += arranged.height - desired_h;
                arranged.height = desired_h;
                break;
            case JI_ALIGN_V_STRETCH:
            default:
                break;
        }
    }

    /* Call the virtual arrange override */
    if (elem->arrange_override) {
        arranged = elem->arrange_override(elem, arranged);
    }

    elem->layout_slot = arranged;
    elem->layout_flags &= ~JI_LAYOUT_ARRANGE_DIRTY;
    elem->layout_flags &= ~JI_LAYOUT_IS_ARRANGING;
}

void ji_layout_element_invalidate_measure(JiLayoutElement* elem) {
    if (!elem) return;
    elem->layout_flags |= JI_LAYOUT_MEASURE_DIRTY;
    /* Invalidate arrange too (measure affects arrange) */
    elem->layout_flags |= JI_LAYOUT_ARRANGE_DIRTY;
    /* Propagate up to parent */
    if (elem->layout_parent) {
        ji_layout_element_invalidate_measure(elem->layout_parent);
    }
}

void ji_layout_element_invalidate_arrange(JiLayoutElement* elem) {
    if (!elem) return;
    elem->layout_flags |= JI_LAYOUT_ARRANGE_DIRTY;
}

void ji_layout_element_update_layout(JiLayoutElement* elem) {
    if (!elem) return;
    if (elem->layout_flags & JI_LAYOUT_MEASURE_DIRTY) {
        ji_layout_element_measure(elem, elem->previous_available_size);
    }
    if (elem->layout_flags & JI_LAYOUT_ARRANGE_DIRTY) {
        ji_layout_element_arrange(elem, elem->layout_slot);
    }
}

/* =========================================================================
 * Getters
 * ========================================================================= */
JiSize ji_layout_element_get_desired_size(const JiLayoutElement* elem) {
    return elem ? elem->desired_size : ji_size(0, 0);
}

JiSize ji_layout_element_get_render_size(const JiLayoutElement* elem) {
    return elem ? ji_size(elem->layout_slot.width, elem->layout_slot.height) : ji_size(0, 0);
}

JiRect ji_layout_element_get_layout_slot(const JiLayoutElement* elem) {
    return elem ? elem->layout_slot : ji_rect(0, 0, 0, 0);
}

bool ji_layout_element_is_measure_dirty(const JiLayoutElement* elem) {
    return elem ? (elem->layout_flags & JI_LAYOUT_MEASURE_DIRTY) != 0 : false;
}

bool ji_layout_element_is_arrange_dirty(const JiLayoutElement* elem) {
    return elem ? (elem->layout_flags & JI_LAYOUT_ARRANGE_DIRTY) != 0 : false;
}

/* =========================================================================
 * Size helpers
 * ========================================================================= */
JiSize ji_layout_element_clamp_size(const JiLayoutElement* elem, JiSize size) {
    if (!elem) return size;
    double w = size.width;
    double h = size.height;
    if (!ji_is_nan(elem->width))  w = elem->width;
    if (!ji_is_nan(elem->height)) h = elem->height;
    if (w < elem->min_width)  w = elem->min_width;
    if (w > elem->max_width)  w = elem->max_width;
    if (h < elem->min_height) h = elem->min_height;
    if (h > elem->max_height) h = elem->max_height;
    return ji_size(w, h);
}

JiSize ji_layout_element_collapse_margin(const JiLayoutElement* elem, JiSize available) {
    if (!elem) return available;
    double w = available.width  - elem->margin.left - elem->margin.right;
    double h = available.height - elem->margin.top  - elem->margin.bottom;
    if (w < 0.0) w = 0.0;
    if (h < 0.0) h = 0.0;
    return ji_size(w, h);
}

JiSize ji_layout_element_expand_margin(const JiLayoutElement* elem, JiSize size) {
    if (!elem) return size;
    double w = size.width  + elem->margin.left + elem->margin.right;
    double h = size.height + elem->margin.top  + elem->margin.bottom;
    return ji_size(w, h);
}

JiSize ji_layout_element_collapse_padding(const JiLayoutElement* elem, JiSize available) {
    if (!elem) return available;
    double w = available.width  - elem->padding.left - elem->padding.right;
    double h = available.height - elem->padding.top  - elem->padding.bottom;
    if (w < 0.0) w = 0.0;
    if (h < 0.0) h = 0.0;
    return ji_size(w, h);
}

JiSize ji_layout_element_expand_padding(const JiLayoutElement* elem, JiSize size) {
    if (!elem) return size;
    double w = size.width  + elem->padding.left + elem->padding.right;
    double h = size.height + elem->padding.top  + elem->padding.bottom;
    return ji_size(w, h);
}

/* =========================================================================
 * Layout type registration
 * ========================================================================= */
void ji_layout_type_init(void) {
    s_layout_element_type = ji_type_register("JiLayoutElement",
        sizeof(JiLayoutElement), JI_TYPE_OBJECT, NULL, NULL);
}
