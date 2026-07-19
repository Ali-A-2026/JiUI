/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input_pen.c
 * @brief Pen/stylus input handling — pressure, tilt, eraser detection.
 */

#include "jiui/ji_api.h"
#include "ji_input_internal.h"
#include "jiui/ji_memory.h"
#include <string.h>

JI_API void ji_input_manager_pen_down(JiInputManager* mgr, float x, float y,
                                  float pressure, float tilt_x, float tilt_y,
                                  bool eraser, uint64_t ts) {
    if (!mgr) return;
    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_PEN_DOWN;
    ev.timestamp = ts;
    ev.x = x;
    ev.y = y;
    ev.pressure = pressure;
    ev.tilt_x = tilt_x;
    ev.tilt_y = tilt_y;
    ev.eraser = eraser;
    ev.pressed = true;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_pen_move(JiInputManager* mgr, float x, float y,
                                  float pressure, float tilt_x, float tilt_y,
                                  bool eraser, uint64_t ts) {
    if (!mgr) return;
    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_PEN_MOVE;
    ev.timestamp = ts;
    ev.x = x;
    ev.y = y;
    ev.pressure = pressure;
    ev.tilt_x = tilt_x;
    ev.tilt_y = tilt_y;
    ev.eraser = eraser;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_pen_up(JiInputManager* mgr, float x, float y, uint64_t ts) {
    if (!mgr) return;
    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_PEN_UP;
    ev.timestamp = ts;
    ev.x = x;
    ev.y = y;
    ev.pressed = false;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API bool ji_input_manager_pen_is_eraser(const JiInputManager* mgr) {
    (void)mgr;
    /* This would track current pen state in a real implementation */
    return false;
}

JI_API float ji_input_manager_pen_get_pressure(const JiInputManager* mgr) {
    (void)mgr;
    return 0.0f;
}

JI_API void ji_input_manager_pen_get_tilt(const JiInputManager* mgr, float* tilt_x, float* tilt_y) {
    if (tilt_x) *tilt_x = 0.0f;
    if (tilt_y) *tilt_y = 0.0f;
}
