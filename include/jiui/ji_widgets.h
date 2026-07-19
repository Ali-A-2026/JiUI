/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_widgets.h
 * @brief Core widgets — Button, Label, TextBox, CheckBox, Slider,
 *        ProgressBar, Image.
 */

#ifndef JIUI_WIDGETS_H
#define JIUI_WIDGETS_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include "ji_drawing.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiButton — clickable button
 * ========================================================================= */
typedef struct JiButton {
    JiControl control;           /* must be first */

    char*   text;                /* button label */
    JiBrush background;         /* normal background */
    JiBrush background_hover;   /* hover background */
    JiBrush background_pressed; /* pressed background */
    JiBrush foreground;         /* text color */

    bool    is_hovered;
    bool    is_pressed;
} JiButton;

/** Create a new button with the given text (copied). */
JI_API JiButton* ji_button_new(const char* text);

/** Destroy a button. */
JI_API void ji_button_destroy(JiButton* btn);

/** Set the button text (copied). */
JI_API void ji_button_set_text(JiButton* btn, const char* text);

/** Get the button text. */
JI_API const char* ji_button_get_text(const JiButton* btn);

/** Get the JiTypeId for JiButton. */
JI_API JiTypeId ji_button_type_id(void);

/* =========================================================================
 * JiLabel — non-interactive text display
 * ========================================================================= */
typedef struct JiLabel {
    JiControl control;           /* must be first */

    char*   text;                /* label text */
    JiBrush foreground;         /* text color */
    double  font_size;
    char*   font_family;        /* font name (copied), NULL = default */
} JiLabel;

/** Create a new label with the given text (copied). */
JI_API JiLabel* ji_label_new(const char* text);

/** Destroy a label. */
JI_API void ji_label_destroy(JiLabel* label);

/** Set the label text (copied). */
JI_API void ji_label_set_text(JiLabel* label, const char* text);

/** Get the label text. */
JI_API const char* ji_label_get_text(const JiLabel* label);

/** Set the font size. */
JI_API void ji_label_set_font_size(JiLabel* label, double size);

/** Get the font size. */
JI_API double ji_label_get_font_size(const JiLabel* label);

/** Get the JiTypeId for JiLabel. */
JI_API JiTypeId ji_label_type_id(void);

/* =========================================================================
 * JiTextBox — editable text input
 * ========================================================================= */
typedef struct JiTextBox {
    JiControl control;           /* must be first */

    char*   text;                /* current text content */
    char*   placeholder;         /* placeholder text when empty */
    JiBrush foreground;
    JiBrush background;
    double  font_size;
    char*   font_family;

    bool    is_read_only;
    bool    is_password;          /* mask characters */
    int     cursor_pos;          /* cursor position in characters */
    int     selection_start;     /* selection range */
    int     selection_end;
    int     max_length;          /* 0 = unlimited */
} JiTextBox;

/** Create a new text box. */
JI_API JiTextBox* ji_text_box_new(void);

/** Destroy a text box. */
JI_API void ji_text_box_destroy(JiTextBox* tb);

/** Set the text (copied). */
JI_API void ji_text_box_set_text(JiTextBox* tb, const char* text);

/** Get the text. */
JI_API const char* ji_text_box_get_text(const JiTextBox* tb);

/** Set placeholder text (copied). */
JI_API void ji_text_box_set_placeholder(JiTextBox* tb, const char* text);

/** Get placeholder text. */
JI_API const char* ji_text_box_get_placeholder(const JiTextBox* tb);

/** Set read-only mode. */
JI_API void ji_text_box_set_read_only(JiTextBox* tb, bool read_only);

/** Get read-only mode. */
JI_API bool ji_text_box_is_read_only(const JiTextBox* tb);

/** Set password mode. */
JI_API void ji_text_box_set_password(JiTextBox* tb, bool password);

/** Get password mode. */
JI_API bool ji_text_box_is_password(const JiTextBox* tb);

/** Set max length (0 = unlimited). */
JI_API void ji_text_box_set_max_length(JiTextBox* tb, int max_len);

/** Get max length. */
JI_API int ji_text_box_get_max_length(const JiTextBox* tb);

/** Get the JiTypeId for JiTextBox. */
JI_API JiTypeId ji_text_box_type_id(void);

/* =========================================================================
 * JiCheckBox — toggle checkbox
 * ========================================================================= */
typedef struct JiCheckBox {
    JiControl control;           /* must be first */

    char*   text;                /* label text */
    bool    is_checked;
    JiBrush foreground;
} JiCheckBox;

/** Create a new checkbox with the given text (copied). */
JI_API JiCheckBox* ji_check_box_new(const char* text);

/** Destroy a checkbox. */
JI_API void ji_check_box_destroy(JiCheckBox* cb);

/** Set the checkbox text (copied). */
JI_API void ji_check_box_set_text(JiCheckBox* cb, const char* text);

/** Get the checkbox text. */
JI_API const char* ji_check_box_get_text(const JiCheckBox* cb);

/** Set the checked state. */
JI_API void ji_check_box_set_checked(JiCheckBox* cb, bool checked);

/** Get the checked state. */
JI_API bool ji_check_box_is_checked(const JiCheckBox* cb);

/** Toggle the checked state. */
JI_API void ji_check_box_toggle(JiCheckBox* cb);

/** Get the JiTypeId for JiCheckBox. */
JI_API JiTypeId ji_check_box_type_id(void);

/* =========================================================================
 * JiSlider — draggable value selector
 * ========================================================================= */
typedef struct JiSlider {
    JiControl control;           /* must be first */

    double  minimum;
    double  maximum;
    double  value;
    double  small_change;        /* step for arrow keys */
    double  large_change;        /* step for page up/down */
    JiBrush track_brush;
    JiBrush thumb_brush;
    bool    is_horizontal;
} JiSlider;

/** Create a new horizontal slider. */
JI_API JiSlider* ji_slider_new(void);

/** Destroy a slider. */
JI_API void ji_slider_destroy(JiSlider* slider);

/** Set the minimum value. */
JI_API void ji_slider_set_minimum(JiSlider* slider, double min);

/** Get the minimum value. */
JI_API double ji_slider_get_minimum(const JiSlider* slider);

/** Set the maximum value. */
JI_API void ji_slider_set_maximum(JiSlider* slider, double max);

/** Get the maximum value. */
JI_API double ji_slider_get_maximum(const JiSlider* slider);

/** Set the current value (clamped to min..max). */
JI_API void ji_slider_set_value(JiSlider* slider, double value);

/** Get the current value. */
JI_API double ji_slider_get_value(const JiSlider* slider);

/** Set the orientation. */
JI_API void ji_slider_set_horizontal(JiSlider* slider, bool horizontal);

/** Get the orientation. */
JI_API bool ji_slider_is_horizontal(const JiSlider* slider);

/** Get the JiTypeId for JiSlider. */
JI_API JiTypeId ji_slider_type_id(void);

/* =========================================================================
 * JiProgressBar — progress indicator
 * ========================================================================= */
typedef struct JiProgressBar {
    JiControl control;           /* must be first */

    double  minimum;
    double  maximum;
    double  value;
    bool    is_indeterminate;    /* shows animation when true */
    JiBrush foreground;
    JiBrush background;
} JiProgressBar;

/** Create a new progress bar. */
JI_API JiProgressBar* ji_progress_bar_new(void);

/** Destroy a progress bar. */
JI_API void ji_progress_bar_destroy(JiProgressBar* pb);

/** Set the minimum value. */
JI_API void ji_progress_bar_set_minimum(JiProgressBar* pb, double min);

/** Get the minimum value. */
JI_API double ji_progress_bar_get_minimum(const JiProgressBar* pb);

/** Set the maximum value. */
JI_API void ji_progress_bar_set_maximum(JiProgressBar* pb, double max);

/** Get the maximum value. */
JI_API double ji_progress_bar_get_maximum(const JiProgressBar* pb);

/** Set the current value (clamped to min..max). */
JI_API void ji_progress_bar_set_value(JiProgressBar* pb, double value);

/** Get the current value. */
JI_API double ji_progress_bar_get_value(const JiProgressBar* pb);

/** Set indeterminate mode. */
JI_API void ji_progress_bar_set_indeterminate(JiProgressBar* pb, bool indeterminate);

/** Get indeterminate mode. */
JI_API bool ji_progress_bar_is_indeterminate(const JiProgressBar* pb);

/** Get the JiTypeId for JiProgressBar. */
JI_API JiTypeId ji_progress_bar_type_id(void);

/* =========================================================================
 * JiImage — image display
 * ========================================================================= */
typedef struct JiImage {
    JiControl control;           /* must be first */

    char*   source;              /* file path or URI (copied) */
    JiBrush stretch;             /* stretch mode (future: use enum) */
    double  width;               /* desired width (0 = auto) */
    double  height;              /* desired height (0 = auto) */
} JiImage;

/** Create a new image with the given source path (copied). */
JI_API JiImage* ji_image_new(const char* source);

/** Destroy an image. */
JI_API void ji_image_destroy(JiImage* img);

/** Set the source path (copied). */
JI_API void ji_image_set_source(JiImage* img, const char* source);

/** Get the source path. */
JI_API const char* ji_image_get_source(const JiImage* img);

/** Get the JiTypeId for JiImage. */
JI_API JiTypeId ji_image_type_id(void);

/* =========================================================================
 * Widget type registration
 * ========================================================================= */

/** Register all widget types (called during ji_initialize). */
JI_API void ji_widgets_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_WIDGETS_H */
