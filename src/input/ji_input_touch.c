/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input_touch.c
 * @brief Touch input handling — multi-touch event dispatch.
 */

#include "jiui/ji_api.h"
#include "ji_input_internal.h"
#include "jiui/ji_memory.h"
#include <string.h>

JI_API void ji_input_manager_touch_down(JiInputManager* mgr, uint32_t touch_id,
                                    float x, float y, float pressure, uint64_t ts) {
    if (!mgr) return;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_TOUCH_DOWN;
    ev.timestamp = ts;
    ev.touch_id = touch_id;
    ev.x = x;
    ev.y = y;
    ev.pressure = pressure;
    ji_input_dispatch_event(mgr, &ev);

    /* Feed gesture manager */
    if (mgr->gesture_mgr) {
        JiTouchPoint pt;
        pt.id = touch_id;
        pt.x = x;
        pt.y = y;
        pt.pressure = pressure;
        pt.timestamp = ts;
        ji_gesture_manager_touch_down(mgr->gesture_mgr, &pt);
    }
}

JI_API void ji_input_manager_touch_move(JiInputManager* mgr, uint32_t touch_id,
                                    float x, float y, float pressure, uint64_t ts) {
    if (!mgr) return;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_TOUCH_MOVE;
    ev.timestamp = ts;
    ev.touch_id = touch_id;
    ev.x = x;
    ev.y = y;
    ev.pressure = pressure;
    ji_input_dispatch_event(mgr, &ev);

    if (mgr->gesture_mgr) {
        JiTouchPoint pt;
        pt.id = touch_id;
        pt.x = x;
        pt.y = y;
        pt.pressure = pressure;
        pt.timestamp = ts;
        ji_gesture_manager_touch_move(mgr->gesture_mgr, &pt);
    }
}

JI_API void ji_input_manager_touch_up(JiInputManager* mgr, uint32_t touch_id, uint64_t ts) {
    if (!mgr) return;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_TOUCH_UP;
    ev.timestamp = ts;
    ev.touch_id = touch_id;
    ji_input_dispatch_event(mgr, &ev);

    if (mgr->gesture_mgr) {
        ji_gesture_manager_touch_up(mgr->gesture_mgr, touch_id, ts);
    }
}

JI_API void ji_input_manager_touch_cancel(JiInputManager* mgr, uint32_t touch_id, uint64_t ts) {
    if (!mgr) return;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_TOUCH_CANCEL;
    ev.timestamp = ts;
    ev.touch_id = touch_id;
    ji_input_dispatch_event(mgr, &ev);

    if (mgr->gesture_mgr) {
        ji_gesture_manager_touch_cancel(mgr->gesture_mgr, ts);
    }
}

JI_API uint32_t ji_input_manager_touch_count(const JiInputManager* mgr) {
    if (!mgr) return 0;
    uint32_t count = 0;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].type == JI_INPUT_DEVICE_TOUCH &&
            mgr->devices[i].state != JI_INPUT_DEVICE_STATE_DISCONNECTED) {
            count++;
        }
    }
    return count;
}

JI_API bool ji_input_manager_has_touch(const JiInputManager* mgr) {
    return ji_input_manager_touch_count(mgr) > 0;
}
