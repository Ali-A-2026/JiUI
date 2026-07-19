/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_widgets.c
 * @brief Implementation of core widgets — Button, Label, TextBox, CheckBox,
 *        Slider, ProgressBar, Image.
 */

#include "jiui/ji_widgets.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_type.h"
#include "jiui/ji_object.h"

#include <string.h>
#include <stdlib.h>

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
 * Static type IDs
 * ========================================================================= */
static JiTypeId s_button_type       = JI_TYPE_INVALID;
static JiTypeId s_label_type        = JI_TYPE_INVALID;
static JiTypeId s_text_box_type     = JI_TYPE_INVALID;
static JiTypeId s_check_box_type    = JI_TYPE_INVALID;
static JiTypeId s_slider_type       = JI_TYPE_INVALID;
static JiTypeId s_progress_bar_type = JI_TYPE_INVALID;
static JiTypeId s_image_type        = JI_TYPE_INVALID;

JiTypeId ji_button_type_id(void)       { return s_button_type; }
JiTypeId ji_label_type_id(void)        { return s_label_type; }
JiTypeId ji_text_box_type_id(void)     { return s_text_box_type; }
JiTypeId ji_check_box_type_id(void)    { return s_check_box_type; }
JiTypeId ji_slider_type_id(void)       { return s_slider_type; }
JiTypeId ji_progress_bar_type_id(void) { return s_progress_bar_type; }
JiTypeId ji_image_type_id(void)        { return s_image_type; }

/* =========================================================================
 * Helper: initialize visual fields for a control-derived struct
 * ========================================================================= */
static void ji_widget_init_visual(JiVisual* v, JiTypeId type_id) {
    JiLayoutElement* layout = &v->layout;
    ji_object_init(&layout->base, type_id);
    layout->horizontal_alignment = JI_ALIGN_H_LEFT;
    layout->vertical_alignment   = JI_ALIGN_V_TOP;
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
}

static void ji_widget_init_control(JiControl* c) {
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
}

/* =========================================================================
 * JiButton
 * ========================================================================= */
static void ji_button_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiButton* btn = (JiButton*)self;
    JiRect slot = self->layout.layout_slot;

    /* Choose background based on state */
    JiBrush* bg = &btn->background;
    if (btn->is_pressed && btn->background_pressed.kind != JI_BRUSH_SOLID) {
        /* use pressed if it's set to something non-default */
    } else if (btn->is_pressed) {
        bg = &btn->background_pressed;
    } else if (btn->is_hovered) {
        bg = &btn->background_hover;
    }

    /* Draw background */
    ji_draw_fill_rect(ctx, slot, bg);

    /* Draw border */
    JiPen border_pen = ji_pen_solid(0xFF333333, 1.0);
    ji_draw_stroke_rect(ctx, slot, &border_pen);
    ji_pen_destroy(&border_pen);

    /* Draw text */
    if (btn->text) {
        JiPoint origin = ji_point(slot.x + 8, slot.y + slot.height / 2);
        ji_draw_text(ctx, origin, btn->text, &btn->foreground, NULL, 14.0);
    }
}

JiButton* ji_button_new(const char* text) {
    JiButton* btn = JI_NEW(JiButton);
    if (!btn) return NULL;
    memset(btn, 0, sizeof(JiButton));

    ji_widget_init_visual(&btn->control.visual, s_button_type);
    ji_widget_init_control(&btn->control);

    btn->text = ji_strdup(text);
    btn->background         = ji_brush_solid(0xFFDDDDDD);
    btn->background_hover   = ji_brush_solid(0xFFEEEEEE);
    btn->background_pressed = ji_brush_solid(0xFFCCCCCC);
    btn->foreground         = ji_brush_solid(0xFF000000);
    btn->is_hovered  = false;
    btn->is_pressed  = false;

    btn->control.visual.on_render = ji_button_on_render;

    return btn;
}

void ji_button_destroy(JiButton* btn) {
    if (!btn) return;
    if (btn->text) ji_free(btn->text);
    ji_brush_destroy(&btn->background);
    ji_brush_destroy(&btn->background_hover);
    ji_brush_destroy(&btn->background_pressed);
    ji_brush_destroy(&btn->foreground);
    ji_visual_destroy(&btn->control.visual);
}

void ji_button_set_text(JiButton* btn, const char* text) {
    if (!btn) return;
    if (btn->text) ji_free(btn->text);
    btn->text = ji_strdup(text);
    ji_visual_invalidate(&btn->control.visual);
}

const char* ji_button_get_text(const JiButton* btn) {
    return btn ? btn->text : NULL;
}

/* =========================================================================
 * JiLabel
 * ========================================================================= */
static void ji_label_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiLabel* label = (JiLabel*)self;
    if (!label->text) return;

    JiRect slot = self->layout.layout_slot;
    JiPoint origin = ji_point(slot.x, slot.y);
    ji_draw_text(ctx, origin, label->text, &label->foreground,
                 label->font_family, label->font_size);
}

JiLabel* ji_label_new(const char* text) {
    JiLabel* label = JI_NEW(JiLabel);
    if (!label) return NULL;
    memset(label, 0, sizeof(JiLabel));

    ji_widget_init_visual(&label->control.visual, s_label_type);
    ji_widget_init_control(&label->control);

    label->text = ji_strdup(text);
    label->foreground  = ji_brush_solid(0xFF000000);
    label->font_size   = 14.0;
    label->font_family = NULL;

    label->control.visual.on_render = ji_label_on_render;

    return label;
}

void ji_label_destroy(JiLabel* label) {
    if (!label) return;
    if (label->text) ji_free(label->text);
    if (label->font_family) ji_free(label->font_family);
    ji_brush_destroy(&label->foreground);
    ji_visual_destroy(&label->control.visual);
}

void ji_label_set_text(JiLabel* label, const char* text) {
    if (!label) return;
    if (label->text) ji_free(label->text);
    label->text = ji_strdup(text);
    ji_visual_invalidate(&label->control.visual);
}

const char* ji_label_get_text(const JiLabel* label) {
    return label ? label->text : NULL;
}

void ji_label_set_font_size(JiLabel* label, double size) {
    if (!label) return;
    label->font_size = size;
    ji_visual_invalidate(&label->control.visual);
}

double ji_label_get_font_size(const JiLabel* label) {
    return label ? label->font_size : 0.0;
}

/* =========================================================================
 * JiTextBox
 * ========================================================================= */
static void ji_text_box_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiTextBox* tb = (JiTextBox*)self;
    JiRect slot = self->layout.layout_slot;

    /* Background */
    ji_draw_fill_rect(ctx, slot, &tb->background);

    /* Border */
    JiPen border_pen = ji_pen_solid(0xFF999999, 1.0);
    ji_draw_stroke_rect(ctx, slot, &border_pen);
    ji_pen_destroy(&border_pen);

    /* Text or placeholder */
    const char* display_text = (tb->text && tb->text[0]) ? tb->text : tb->placeholder;
    if (display_text) {
        JiBrush placeholder_fg = ji_brush_solid(0xFF999999);
        JiBrush* fg = (tb->text && tb->text[0]) ? &tb->foreground : &placeholder_fg;
        JiPoint origin = ji_point(slot.x + 4, slot.y + slot.height / 2);
        ji_draw_text(ctx, origin, display_text, fg, tb->font_family, tb->font_size);
        ji_brush_destroy(&placeholder_fg);
    }

    /* Cursor (if focused) */
    if (tb->control.is_focused) {
        /* Simple cursor indicator — a vertical line at the end of text */
        double cursor_x = slot.x + 4 + (tb->text ? strlen(tb->text) * 8.0 : 0);
        JiPoint c_start = ji_point(cursor_x, slot.y + 4);
        JiPoint c_end   = ji_point(cursor_x, slot.y + slot.height - 4);
        JiBrush cursor_brush = ji_brush_solid(0xFF000000);
        ji_draw_line(ctx, c_start, c_end, &cursor_brush, 1.0);
        ji_brush_destroy(&cursor_brush);
    }
}

JiTextBox* ji_text_box_new(void) {
    JiTextBox* tb = JI_NEW(JiTextBox);
    if (!tb) return NULL;
    memset(tb, 0, sizeof(JiTextBox));

    ji_widget_init_visual(&tb->control.visual, s_text_box_type);
    ji_widget_init_control(&tb->control);

    tb->text        = ji_strdup("");
    tb->placeholder = NULL;
    tb->foreground  = ji_brush_solid(0xFF000000);
    tb->background  = ji_brush_solid(0xFFFFFFFF);
    tb->font_size   = 14.0;
    tb->font_family = NULL;
    tb->is_read_only = false;
    tb->is_password  = false;
    tb->cursor_pos   = 0;
    tb->selection_start = 0;
    tb->selection_end   = 0;
    tb->max_length   = 0;

    tb->control.visual.on_render = ji_text_box_on_render;

    return tb;
}

void ji_text_box_destroy(JiTextBox* tb) {
    if (!tb) return;
    if (tb->text) ji_free(tb->text);
    if (tb->placeholder) ji_free(tb->placeholder);
    if (tb->font_family) ji_free(tb->font_family);
    ji_brush_destroy(&tb->foreground);
    ji_brush_destroy(&tb->background);
    ji_visual_destroy(&tb->control.visual);
}

void ji_text_box_set_text(JiTextBox* tb, const char* text) {
    if (!tb) return;
    if (tb->text) ji_free(tb->text);
    tb->text = ji_strdup(text ? text : "");
    ji_visual_invalidate(&tb->control.visual);
    if (tb->control.on_value_changed) {
        tb->control.on_value_changed(&tb->control, tb->control.on_value_changed_data);
    }
}

const char* ji_text_box_get_text(const JiTextBox* tb) {
    return tb ? tb->text : NULL;
}

void ji_text_box_set_placeholder(JiTextBox* tb, const char* text) {
    if (!tb) return;
    if (tb->placeholder) ji_free(tb->placeholder);
    tb->placeholder = ji_strdup(text);
    ji_visual_invalidate(&tb->control.visual);
}

const char* ji_text_box_get_placeholder(const JiTextBox* tb) {
    return tb ? tb->placeholder : NULL;
}

void ji_text_box_set_read_only(JiTextBox* tb, bool read_only) {
    if (!tb) return;
    tb->is_read_only = read_only;
}

bool ji_text_box_is_read_only(const JiTextBox* tb) {
    return tb ? tb->is_read_only : false;
}

void ji_text_box_set_password(JiTextBox* tb, bool password) {
    if (!tb) return;
    tb->is_password = password;
    ji_visual_invalidate(&tb->control.visual);
}

bool ji_text_box_is_password(const JiTextBox* tb) {
    return tb ? tb->is_password : false;
}

void ji_text_box_set_max_length(JiTextBox* tb, int max_len) {
    if (!tb) return;
    tb->max_length = max_len < 0 ? 0 : max_len;
}

int ji_text_box_get_max_length(const JiTextBox* tb) {
    return tb ? tb->max_length : 0;
}

/* =========================================================================
 * JiCheckBox
 * ========================================================================= */
static void ji_check_box_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiCheckBox* cb = (JiCheckBox*)self;
    JiRect slot = self->layout.layout_slot;

    /* Draw checkbox box */
    JiRect box_rect = ji_rect(slot.x, slot.y + 2, 16, 16);
    JiBrush box_bg = ji_brush_solid(0xFFFFFFFF);
    ji_draw_fill_rect(ctx, box_rect, &box_bg);
    ji_brush_destroy(&box_bg);

    JiPen box_pen = ji_pen_solid(0xFF333333, 1.0);
    ji_draw_stroke_rect(ctx, box_rect, &box_pen);
    ji_pen_destroy(&box_pen);

    /* Draw checkmark if checked */
    if (cb->is_checked) {
        JiBrush check_brush = ji_brush_solid(0xFF0066CC);
        JiRect check_rect = ji_rect(slot.x + 3, slot.y + 5, 10, 10);
        ji_draw_fill_rect(ctx, check_rect, &check_brush);
        ji_brush_destroy(&check_brush);
    }

    /* Draw label text */
    if (cb->text) {
        JiPoint origin = ji_point(slot.x + 22, slot.y + 2);
        ji_draw_text(ctx, origin, cb->text, &cb->foreground, NULL, 14.0);
    }
}

JiCheckBox* ji_check_box_new(const char* text) {
    JiCheckBox* cb = JI_NEW(JiCheckBox);
    if (!cb) return NULL;
    memset(cb, 0, sizeof(JiCheckBox));

    ji_widget_init_visual(&cb->control.visual, s_check_box_type);
    ji_widget_init_control(&cb->control);

    cb->text = ji_strdup(text);
    cb->is_checked = false;
    cb->foreground = ji_brush_solid(0xFF000000);

    cb->control.visual.on_render = ji_check_box_on_render;

    return cb;
}

void ji_check_box_destroy(JiCheckBox* cb) {
    if (!cb) return;
    if (cb->text) ji_free(cb->text);
    ji_brush_destroy(&cb->foreground);
    ji_visual_destroy(&cb->control.visual);
}

void ji_check_box_set_text(JiCheckBox* cb, const char* text) {
    if (!cb) return;
    if (cb->text) ji_free(cb->text);
    cb->text = ji_strdup(text);
    ji_visual_invalidate(&cb->control.visual);
}

const char* ji_check_box_get_text(const JiCheckBox* cb) {
    return cb ? cb->text : NULL;
}

void ji_check_box_set_checked(JiCheckBox* cb, bool checked) {
    if (!cb) return;
    cb->is_checked = checked;
    ji_visual_invalidate(&cb->control.visual);
    if (cb->control.on_value_changed) {
        cb->control.on_value_changed(&cb->control, cb->control.on_value_changed_data);
    }
}

bool ji_check_box_is_checked(const JiCheckBox* cb) {
    return cb ? cb->is_checked : false;
}

void ji_check_box_toggle(JiCheckBox* cb) {
    if (!cb) return;
    ji_check_box_set_checked(cb, !cb->is_checked);
}

/* =========================================================================
 * JiSlider
 * ========================================================================= */
static void ji_slider_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiSlider* slider = (JiSlider*)self;
    JiRect slot = self->layout.layout_slot;

    double range = slider->maximum - slider->minimum;
    double ratio = range > 0.0 ? (slider->value - slider->minimum) / range : 0.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    if (slider->is_horizontal) {
        /* Track */
        double track_y = slot.y + slot.height / 2 - 3;
        JiRect track_rect = ji_rect(slot.x, track_y, slot.width, 6);
        ji_draw_fill_rect(ctx, track_rect, &slider->track_brush);

        /* Filled portion */
        double filled_w = slot.width * ratio;
        JiRect filled_rect = ji_rect(slot.x, track_y, filled_w, 6);
        ji_draw_fill_rect(ctx, filled_rect, &slider->thumb_brush);

        /* Thumb */
        double thumb_x = slot.x + filled_w - 6;
        JiRect thumb_rect = ji_rect(thumb_x, track_y - 6, 12, 18);
        ji_draw_fill_rect(ctx, thumb_rect, &slider->thumb_brush);
    } else {
        /* Vertical */
        double track_x = slot.x + slot.width / 2 - 3;
        JiRect track_rect = ji_rect(track_x, slot.y, 6, slot.height);
        ji_draw_fill_rect(ctx, track_rect, &slider->track_brush);

        double filled_h = slot.height * ratio;
        JiRect filled_rect = ji_rect(track_x, slot.y + slot.height - filled_h, 6, filled_h);
        ji_draw_fill_rect(ctx, filled_rect, &slider->thumb_brush);

        double thumb_y = slot.y + slot.height - filled_h - 6;
        JiRect thumb_rect = ji_rect(track_x - 6, thumb_y, 18, 12);
        ji_draw_fill_rect(ctx, thumb_rect, &slider->thumb_brush);
    }
}

JiSlider* ji_slider_new(void) {
    JiSlider* slider = JI_NEW(JiSlider);
    if (!slider) return NULL;
    memset(slider, 0, sizeof(JiSlider));

    ji_widget_init_visual(&slider->control.visual, s_slider_type);
    ji_widget_init_control(&slider->control);

    slider->minimum      = 0.0;
    slider->maximum      = 100.0;
    slider->value         = 0.0;
    slider->small_change  = 1.0;
    slider->large_change  = 10.0;
    slider->track_brush   = ji_brush_solid(0xFFCCCCCC);
    slider->thumb_brush  = ji_brush_solid(0xFF0066CC);
    slider->is_horizontal = true;

    slider->control.visual.on_render = ji_slider_on_render;

    return slider;
}

void ji_slider_destroy(JiSlider* slider) {
    if (!slider) return;
    ji_brush_destroy(&slider->track_brush);
    ji_brush_destroy(&slider->thumb_brush);
    ji_visual_destroy(&slider->control.visual);
}

void ji_slider_set_minimum(JiSlider* slider, double min) {
    if (!slider) return;
    slider->minimum = min;
    if (slider->value < min) slider->value = min;
    ji_visual_invalidate(&slider->control.visual);
}

double ji_slider_get_minimum(const JiSlider* slider) {
    return slider ? slider->minimum : 0.0;
}

void ji_slider_set_maximum(JiSlider* slider, double max) {
    if (!slider) return;
    slider->maximum = max;
    if (slider->value > max) slider->value = max;
    ji_visual_invalidate(&slider->control.visual);
}

double ji_slider_get_maximum(const JiSlider* slider) {
    return slider ? slider->maximum : 100.0;
}

void ji_slider_set_value(JiSlider* slider, double value) {
    if (!slider) return;
    if (value < slider->minimum) value = slider->minimum;
    if (value > slider->maximum) value = slider->maximum;
    slider->value = value;
    ji_visual_invalidate(&slider->control.visual);
    if (slider->control.on_value_changed) {
        slider->control.on_value_changed(&slider->control, slider->control.on_value_changed_data);
    }
}

double ji_slider_get_value(const JiSlider* slider) {
    return slider ? slider->value : 0.0;
}

void ji_slider_set_horizontal(JiSlider* slider, bool horizontal) {
    if (!slider) return;
    slider->is_horizontal = horizontal;
    ji_visual_invalidate(&slider->control.visual);
}

bool ji_slider_is_horizontal(const JiSlider* slider) {
    return slider ? slider->is_horizontal : true;
}

/* =========================================================================
 * JiProgressBar
 * ========================================================================= */
static void ji_progress_bar_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiProgressBar* pb = (JiProgressBar*)self;
    JiRect slot = self->layout.layout_slot;

    /* Background track */
    ji_draw_fill_rect(ctx, slot, &pb->background);

    /* Filled portion */
    if (!pb->is_indeterminate) {
        double range = pb->maximum - pb->minimum;
        double ratio = range > 0.0 ? (pb->value - pb->minimum) / range : 0.0;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
        double filled_w = slot.width * ratio;
        JiRect filled_rect = ji_rect(slot.x, slot.y, filled_w, slot.height);
        ji_draw_fill_rect(ctx, filled_rect, &pb->foreground);
    } else {
        /* Indeterminate: animated stripe (simplified as a moving block) */
        double block_w = slot.width * 0.3;
        JiRect block = ji_rect(slot.x, slot.y, block_w, slot.height);
        ji_draw_fill_rect(ctx, block, &pb->foreground);
    }

    /* Border */
    JiPen border_pen = ji_pen_solid(0xFF666666, 1.0);
    ji_draw_stroke_rect(ctx, slot, &border_pen);
    ji_pen_destroy(&border_pen);
}

JiProgressBar* ji_progress_bar_new(void) {
    JiProgressBar* pb = JI_NEW(JiProgressBar);
    if (!pb) return NULL;
    memset(pb, 0, sizeof(JiProgressBar));

    ji_widget_init_visual(&pb->control.visual, s_progress_bar_type);
    ji_widget_init_control(&pb->control);

    pb->minimum        = 0.0;
    pb->maximum        = 100.0;
    pb->value          = 0.0;
    pb->is_indeterminate = false;
    pb->foreground     = ji_brush_solid(0xFF0066CC);
    pb->background     = ji_brush_solid(0xFFDDDDDD);

    pb->control.visual.on_render = ji_progress_bar_on_render;

    return pb;
}

void ji_progress_bar_destroy(JiProgressBar* pb) {
    if (!pb) return;
    ji_brush_destroy(&pb->foreground);
    ji_brush_destroy(&pb->background);
    ji_visual_destroy(&pb->control.visual);
}

void ji_progress_bar_set_minimum(JiProgressBar* pb, double min) {
    if (!pb) return;
    pb->minimum = min;
    if (pb->value < min) pb->value = min;
    ji_visual_invalidate(&pb->control.visual);
}

double ji_progress_bar_get_minimum(const JiProgressBar* pb) {
    return pb ? pb->minimum : 0.0;
}

void ji_progress_bar_set_maximum(JiProgressBar* pb, double max) {
    if (!pb) return;
    pb->maximum = max;
    if (pb->value > max) pb->value = max;
    ji_visual_invalidate(&pb->control.visual);
}

double ji_progress_bar_get_maximum(const JiProgressBar* pb) {
    return pb ? pb->maximum : 100.0;
}

void ji_progress_bar_set_value(JiProgressBar* pb, double value) {
    if (!pb) return;
    if (value < pb->minimum) value = pb->minimum;
    if (value > pb->maximum) value = pb->maximum;
    pb->value = value;
    ji_visual_invalidate(&pb->control.visual);
    if (pb->control.on_value_changed) {
        pb->control.on_value_changed(&pb->control, pb->control.on_value_changed_data);
    }
}

double ji_progress_bar_get_value(const JiProgressBar* pb) {
    return pb ? pb->value : 0.0;
}

void ji_progress_bar_set_indeterminate(JiProgressBar* pb, bool indeterminate) {
    if (!pb) return;
    pb->is_indeterminate = indeterminate;
    ji_visual_invalidate(&pb->control.visual);
}

bool ji_progress_bar_is_indeterminate(const JiProgressBar* pb) {
    return pb ? pb->is_indeterminate : false;
}

/* =========================================================================
 * JiImage
 * ========================================================================= */
static void ji_image_on_render(JiVisual* self, JiDrawingContext* ctx) {
    JiImage* img = (JiImage*)self;
    (void)img;
    /* Actual image rendering requires a texture/surface backend.
       For now, draw a placeholder rectangle. */
    JiRect slot = self->layout.layout_slot;
    JiBrush placeholder = ji_brush_solid(0xFFEEEEEE);
    ji_draw_fill_rect(ctx, slot, &placeholder);
    ji_brush_destroy(&placeholder);

    JiPen border_pen = ji_pen_solid(0xFFCCCCCC, 1.0);
    ji_draw_stroke_rect(ctx, slot, &border_pen);
    ji_pen_destroy(&border_pen);

    if (img->source) {
        JiBrush text_brush = ji_brush_solid(0xFF999999);
        JiPoint origin = ji_point(slot.x + 4, slot.y + slot.height / 2);
        ji_draw_text(ctx, origin, img->source, &text_brush, NULL, 10.0);
        ji_brush_destroy(&text_brush);
    }
}

JiImage* ji_image_new(const char* source) {
    JiImage* img = JI_NEW(JiImage);
    if (!img) return NULL;
    memset(img, 0, sizeof(JiImage));

    ji_widget_init_visual(&img->control.visual, s_image_type);
    ji_widget_init_control(&img->control);

    img->source = ji_strdup(source);
    img->stretch = ji_brush_solid(0x00000000);
    img->width   = 0.0;
    img->height  = 0.0;

    img->control.visual.on_render = ji_image_on_render;

    return img;
}

void ji_image_destroy(JiImage* img) {
    if (!img) return;
    if (img->source) ji_free(img->source);
    ji_brush_destroy(&img->stretch);
    ji_visual_destroy(&img->control.visual);
}

void ji_image_set_source(JiImage* img, const char* source) {
    if (!img) return;
    if (img->source) ji_free(img->source);
    img->source = ji_strdup(source);
    ji_visual_invalidate(&img->control.visual);
}

const char* ji_image_get_source(const JiImage* img) {
    return img ? img->source : NULL;
}

/* =========================================================================
 * Widget type registration
 * ========================================================================= */
void ji_widgets_type_init(void) {
    s_button_type = ji_type_register("JiButton", sizeof(JiButton),
                                      ji_control_type_id(), NULL, NULL);
    s_label_type = ji_type_register("JiLabel", sizeof(JiLabel),
                                      ji_control_type_id(), NULL, NULL);
    s_text_box_type = ji_type_register("JiTextBox", sizeof(JiTextBox),
                                        ji_control_type_id(), NULL, NULL);
    s_check_box_type = ji_type_register("JiCheckBox", sizeof(JiCheckBox),
                                          ji_control_type_id(), NULL, NULL);
    s_slider_type = ji_type_register("JiSlider", sizeof(JiSlider),
                                      ji_control_type_id(), NULL, NULL);
    s_progress_bar_type = ji_type_register("JiProgressBar", sizeof(JiProgressBar),
                                            ji_control_type_id(), NULL, NULL);
    s_image_type = ji_type_register("JiImage", sizeof(JiImage),
                                      ji_control_type_id(), NULL, NULL);
}
