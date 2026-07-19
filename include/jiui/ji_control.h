/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_control.h
 * @brief Base control class — focus, enabled state, tooltip, tab order, name.
 *
 * JiControl extends JiVisual with input handling concepts. All interactive
 * widgets (Button, TextBox, CheckBox, etc.) derive from JiControl.
 */

#ifndef JIUI_CONTROL_H
#define JIUI_CONTROL_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_visual.h"
#include "ji_drawing.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiControl — base class for interactive widgets
 * ========================================================================= */
typedef struct JiControl JiControl;

/** Callback for when the control is clicked/activated. */
typedef void (*JiControlClickedCallback)(JiControl* sender, void* user_data);

/** Callback for when a property changes on the control. */
typedef void (*JiControlChangedCallback)(JiControl* sender, void* user_data);

struct JiControl {
    JiVisual visual;              /* must be first */

    /* Control state */
    bool     is_enabled;
    bool     is_focusable;
    bool     is_focused;
    int      tab_index;

    /* Identity */
    char*    name;               /* optional name for lookup */

    /* Tooltip */
    char*    tooltip_text;

    /* Template / content */
    JiVisual*  template_root;    /* visual tree for the control's appearance */

    /* Callbacks */
    JiControlClickedCallback on_clicked;
    void*                    on_clicked_data;
    JiControlChangedCallback  on_value_changed;
    void*                    on_value_changed_data;
};

/* ---- Lifetime ---- */

/** Create a new control. */
JI_API JiControl* ji_control_new(void);

/** Destroy a control. */
JI_API void ji_control_destroy(JiControl* control);

/* ---- Properties ---- */

/** Set whether the control is enabled. */
JI_API void ji_control_set_enabled(JiControl* control, bool enabled);

/** Get whether the control is enabled. */
JI_API bool ji_control_is_enabled(const JiControl* control);

/** Set whether the control can receive focus. */
JI_API void ji_control_set_focusable(JiControl* control, bool focusable);

/** Get whether the control can receive focus. */
JI_API bool ji_control_is_focusable(const JiControl* control);

/** Set whether the control has focus. */
JI_API void ji_control_set_focused(JiControl* control, bool focused);

/** Get whether the control has focus. */
JI_API bool ji_control_is_focused(const JiControl* control);

/** Set the tab index. */
JI_API void ji_control_set_tab_index(JiControl* control, int index);

/** Get the tab index. */
JI_API int ji_control_get_tab_index(const JiControl* control);

/** Set the control name (string is copied). */
JI_API void ji_control_set_name(JiControl* control, const char* name);

/** Get the control name. */
JI_API const char* ji_control_get_name(const JiControl* control);

/** Set the tooltip text (string is copied). */
JI_API void ji_control_set_tooltip(JiControl* control, const char* text);

/** Get the tooltip text. */
JI_API const char* ji_control_get_tooltip(const JiControl* control);

/** Set the clicked callback. */
JI_API void ji_control_set_on_clicked(JiControl* control,
                                       JiControlClickedCallback cb, void* user_data);

/** Set the value-changed callback. */
JI_API void ji_control_set_on_value_changed(JiControl* control,
                                              JiControlChangedCallback cb, void* user_data);

/** Get the JiTypeId for JiControl. */
JI_API JiTypeId ji_control_type_id(void);

/** Initialize the control type (called during ji_initialize). */
JI_API void ji_control_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_CONTROL_H */
