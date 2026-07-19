/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_control.c
 * @brief Implementation of the base control class.
 */

#include "jiui/ji_control.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_type.h"
#include "jiui/ji_object.h"

#include <string.h>
#include <stdlib.h>

/* =========================================================================
 * Static type ID
 * ========================================================================= */
static JiTypeId s_control_type = JI_TYPE_INVALID;

JiTypeId ji_control_type_id(void) { return s_control_type; }

/* =========================================================================
 * Helper: strdup using ji_alloc
 * ========================================================================= */
static char* ji_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)ji_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

/* =========================================================================
 * Lifetime
 * ========================================================================= */
JiControl* ji_control_new(void) {
    JiControl* c = JI_NEW(JiControl);
    if (!c) return NULL;
    memset(c, 0, sizeof(JiControl));

    /* Initialize visual (first field) */
    JiVisual* v = &c->visual;
    JiLayoutElement* layout = &v->layout;
    ji_object_init(&layout->base, s_control_type);
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

    /* Control-specific defaults */
    c->is_enabled   = true;
    c->is_focusable = true;
    c->is_focused   = false;
    c->tab_index    = 0;
    c->name         = NULL;
    c->tooltip_text = NULL;
    c->template_root = NULL;
    c->on_clicked       = NULL;
    c->on_clicked_data  = NULL;
    c->on_value_changed = NULL;
    c->on_value_changed_data = NULL;

    return c;
}

void ji_control_destroy(JiControl* control) {
    if (!control) return;
    if (control->name)         ji_free(control->name);
    if (control->tooltip_text) ji_free(control->tooltip_text);
    /* template_root is a visual child, destroyed by visual tree */
    ji_visual_destroy(&control->visual);
}

/* =========================================================================
 * Properties
 * ========================================================================= */
void ji_control_set_enabled(JiControl* control, bool enabled) {
    if (!control) return;
    control->is_enabled = enabled;
    ji_visual_invalidate(&control->visual);
}

bool ji_control_is_enabled(const JiControl* control) {
    return control ? control->is_enabled : false;
}

void ji_control_set_focusable(JiControl* control, bool focusable) {
    if (!control) return;
    control->is_focusable = focusable;
}

bool ji_control_is_focusable(const JiControl* control) {
    return control ? control->is_focusable : false;
}

void ji_control_set_focused(JiControl* control, bool focused) {
    if (!control) return;
    control->is_focused = focused;
    ji_visual_invalidate(&control->visual);
}

bool ji_control_is_focused(const JiControl* control) {
    return control ? control->is_focused : false;
}

void ji_control_set_tab_index(JiControl* control, int index) {
    if (!control) return;
    control->tab_index = index;
}

int ji_control_get_tab_index(const JiControl* control) {
    return control ? control->tab_index : 0;
}

void ji_control_set_name(JiControl* control, const char* name) {
    if (!control) return;
    if (control->name) ji_free(control->name);
    control->name = ji_strdup(name);
}

const char* ji_control_get_name(const JiControl* control) {
    return control ? control->name : NULL;
}

void ji_control_set_tooltip(JiControl* control, const char* text) {
    if (!control) return;
    if (control->tooltip_text) ji_free(control->tooltip_text);
    control->tooltip_text = ji_strdup(text);
}

const char* ji_control_get_tooltip(const JiControl* control) {
    return control ? control->tooltip_text : NULL;
}

void ji_control_set_on_clicked(JiControl* control,
                                JiControlClickedCallback cb, void* user_data) {
    if (!control) return;
    control->on_clicked = cb;
    control->on_clicked_data = user_data;
}

void ji_control_set_on_value_changed(JiControl* control,
                                       JiControlChangedCallback cb, void* user_data) {
    if (!control) return;
    control->on_value_changed = cb;
    control->on_value_changed_data = user_data;
}

/* =========================================================================
 * Type registration
 * ========================================================================= */
void ji_control_type_init(void) {
    s_control_type = ji_type_register("JiControl", sizeof(JiControl),
                                       ji_visual_type_id(), NULL, NULL);
}
