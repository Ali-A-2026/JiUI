/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_auto_hide.c
 * @brief Auto-hide dock animation helpers.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_dock_pro.h"
#include "jiui/ji_memory.h"
#include <math.h>

/* Default animation duration in seconds */
#define JI_AUTO_HIDE_DURATION 0.25f

/* Easing function (ease-in-out cubic) */
static float ease_in_out_cubic(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    float f = 2.0f * t - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}

/* Compute interpolated rect between hidden and shown */
JiRect ji_auto_hide_dock_interpolate_rect(const JiAutoHideDock* dock, float progress) {
    if (!dock) {
        JiRect empty = {0, 0, 0, 0};
        return empty;
    }

    float t = ease_in_out_cubic(progress);
    JiRect result;
    result.x = (int)(dock->hidden_rect.x + (dock->shown_rect.x - dock->hidden_rect.x) * t);
    result.y = (int)(dock->hidden_rect.y + (dock->shown_rect.y - dock->hidden_rect.y) * t);
    result.width = (int)(dock->hidden_rect.width + (dock->shown_rect.width - dock->hidden_rect.width) * t);
    result.height = (int)(dock->hidden_rect.height + (dock->shown_rect.height - dock->hidden_rect.height) * t);
    return result;
}

/* Set the hidden and shown geometry for an auto-hide dock */
void ji_auto_hide_dock_set_geometry(JiAutoHideDock* dock, JiRect hidden, JiRect shown) {
    if (!dock) return;
    dock->hidden_rect = hidden;
    dock->shown_rect = shown;
}

/* Get the current interpolated geometry */
JiRect ji_auto_hide_dock_current_rect(const JiAutoHideDock* dock) {
    return ji_auto_hide_dock_interpolate_rect(dock, dock ? dock->animation_progress : 0.0f);
}

/* Check if mouse is hovering over the dock's trigger zone */
bool ji_auto_hide_dock_is_hovered(const JiAutoHideDock* dock, int mouse_x, int mouse_y) {
    if (!dock) return false;
    JiRect trigger = dock->hidden_rect;
    return (mouse_x >= trigger.x && mouse_x < trigger.x + trigger.width &&
            mouse_y >= trigger.y && mouse_y < trigger.y + trigger.height);
}

/* Get animation progress (0.0 = hidden, 1.0 = shown) */
float ji_auto_hide_dock_progress(const JiAutoHideDock* dock) {
    return dock ? dock->animation_progress : 0.0f;
}

/* Check if animation is complete */
bool ji_auto_hide_dock_is_animating(const JiAutoHideDock* dock) {
    return dock && dock->state == JI_DOCK_AUTO_HIDE_ANIMATING;
}
