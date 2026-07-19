/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input.c
 * @brief Advanced input engine implementation.
 */

#include "jiui/ji_api.h"
#include "ji_input_internal.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>

static void input_ensure_device_capacity(JiInputManager* mgr) {
    if (mgr->device_count >= mgr->device_capacity) {
        mgr->device_capacity = mgr->device_capacity ? mgr->device_capacity * 2 : 8;
        mgr->devices = ji_realloc(mgr->devices, mgr->device_capacity * sizeof(JiInputDevice));
    }
}

static void input_ensure_key_capacity(JiInputManager* mgr, uint32_t key) {
    if (key >= mgr->key_capacity) {
        uint32_t new_cap = mgr->key_capacity ? mgr->key_capacity : 256;
        while (new_cap <= key) new_cap *= 2;
        mgr->key_states = ji_realloc(mgr->key_states, new_cap * sizeof(bool));
        memset(mgr->key_states + mgr->key_capacity, 0,
               (new_cap - mgr->key_capacity) * sizeof(bool));
        mgr->key_capacity = new_cap;
    }
}

static void input_ensure_gamepad_capacity(JiInputManager* mgr, uint32_t id) {
    if (id >= mgr->gamepad_state_capacity) {
        uint32_t new_cap = mgr->gamepad_state_capacity ? mgr->gamepad_state_capacity * 2 : 4;
        while (new_cap <= id) new_cap *= 2;
        mgr->gamepad_states = ji_realloc(mgr->gamepad_states, new_cap * sizeof(GamepadState));
        memset(mgr->gamepad_states + mgr->gamepad_state_capacity, 0,
               (new_cap - mgr->gamepad_state_capacity) * sizeof(GamepadState));
        mgr->gamepad_state_capacity = new_cap;
    }
    if (id >= mgr->gamepad_state_count) mgr->gamepad_state_count = id + 1;
}

JI_API JiInputManager* ji_input_manager_new(void) {
    JiInputManager* mgr = ji_calloc(1, sizeof(JiInputManager));
    if (!mgr) return NULL;
    mgr->gesture_mgr = ji_gesture_manager_new();
    mgr->owns_gesture_mgr = true;
    return mgr;
}

JI_API void ji_input_manager_destroy(JiInputManager* mgr) {
    if (!mgr) return;
    if (mgr->owns_gesture_mgr && mgr->gesture_mgr) {
        ji_gesture_manager_destroy(mgr->gesture_mgr);
    }
    ji_free(mgr->devices);
    ji_free(mgr->key_states);
    ji_free(mgr->gamepad_states);
    ji_free(mgr);
}

JI_API void ji_input_manager_set_callback(JiInputManager* mgr,
                                     JiInputCallback callback,
                                     void* user_data) {
    if (mgr) {
        mgr->global_callback = callback;
        mgr->global_user_data = user_data;
    }
}

JI_API void ji_input_manager_set_device_callback(JiInputManager* mgr,
                                            JiInputDeviceType type,
                                            JiInputCallback callback,
                                            void* user_data) {
    if (mgr && type < 9) {
        mgr->device_callbacks[type] = callback;
        mgr->device_user_data[type] = user_data;
    }
}

/* =========================================================================
 * Device Enumeration
 * ========================================================================= */

JI_API uint32_t ji_input_manager_device_count(const JiInputManager* mgr) {
    return mgr ? mgr->device_count : 0;
}

JI_API bool ji_input_manager_get_device(const JiInputManager* mgr, uint32_t index,
                                   JiInputDevice* out_device) {
    if (!mgr || index >= mgr->device_count || !out_device) return false;
    *out_device = mgr->devices[index];
    return true;
}

JI_API bool ji_input_manager_get_device_by_id(const JiInputManager* mgr, uint32_t id,
                                        JiInputDevice* out_device) {
    if (!mgr || !out_device) return false;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].id == id) {
            *out_device = mgr->devices[i];
            return true;
        }
    }
    return false;
}

JI_API uint32_t ji_input_manager_find_devices(const JiInputManager* mgr,
                                          JiInputDeviceType type,
                                          JiInputDevice* out_devices,
                                          uint32_t max_count) {
    if (!mgr || !out_devices) return 0;
    uint32_t found = 0;
    for (uint32_t i = 0; i < mgr->device_count && found < max_count; i++) {
        if (mgr->devices[i].type == type) {
            out_devices[found++] = mgr->devices[i];
        }
    }
    return found;
}

/* =========================================================================
 * Event Dispatch
 * ========================================================================= */

JI_API void ji_input_dispatch_event(JiInputManager* mgr, const JiInputEvent* event) {
    if (!mgr || !event) return;

    /* Global callback */
    if (mgr->global_callback) {
        mgr->global_callback(event, mgr->global_user_data);
    }

    /* Device-specific callback */
    JiInputDeviceType dt = JI_INPUT_DEVICE_NONE;
    switch (event->type) {
        case JI_INPUT_EVENT_MOUSE_MOVE:
        case JI_INPUT_EVENT_MOUSE_BUTTON:
        case JI_INPUT_EVENT_MOUSE_WHEEL:
            dt = JI_INPUT_DEVICE_MOUSE;
            break;
        case JI_INPUT_EVENT_TOUCH_DOWN:
        case JI_INPUT_EVENT_TOUCH_MOVE:
        case JI_INPUT_EVENT_TOUCH_UP:
        case JI_INPUT_EVENT_TOUCH_CANCEL:
            dt = JI_INPUT_DEVICE_TOUCH;
            break;
        case JI_INPUT_EVENT_PEN_DOWN:
        case JI_INPUT_EVENT_PEN_MOVE:
        case JI_INPUT_EVENT_PEN_UP:
            dt = JI_INPUT_DEVICE_PEN;
            break;
        case JI_INPUT_EVENT_GAMEPAD_BUTTON:
        case JI_INPUT_EVENT_GAMEPAD_AXIS:
            dt = JI_INPUT_DEVICE_GAMEPAD;
            break;
        default:
            break;
    }
    if (dt < 9 && mgr->device_callbacks[dt]) {
        mgr->device_callbacks[dt](event, mgr->device_user_data[dt]);
    }
}

JI_API void ji_input_manager_dispatch_event(JiInputManager* mgr, const JiInputEvent* event) {
    ji_input_dispatch_event(mgr, event);
}

JI_API void ji_input_manager_mouse_move(JiInputManager* mgr, float x, float y, uint64_t ts) {
    if (!mgr) return;
    float dx = x - mgr->mouse_state.x;
    float dy = y - mgr->mouse_state.y;
    mgr->mouse_state.x = x;
    mgr->mouse_state.y = y;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_MOUSE_MOVE;
    ev.timestamp = ts;
    ev.x = x;
    ev.y = y;
    ev.dx = dx;
    ev.dy = dy;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_mouse_button(JiInputManager* mgr, uint32_t button,
                                     bool pressed, uint64_t ts) {
    if (!mgr) return;
    if (pressed) {
        mgr->mouse_state.buttons |= button;
    } else {
        mgr->mouse_state.buttons &= ~button;
    }

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_MOUSE_BUTTON;
    ev.timestamp = ts;
    ev.button = button;
    ev.pressed = pressed;
    ev.x = mgr->mouse_state.x;
    ev.y = mgr->mouse_state.y;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_mouse_wheel(JiInputManager* mgr, float dx, float dy, uint64_t ts) {
    if (!mgr) return;
    mgr->mouse_state.wheel_delta_x = dx;
    mgr->mouse_state.wheel_delta_y = dy;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_MOUSE_WHEEL;
    ev.timestamp = ts;
    ev.wheel_x = dx;
    ev.wheel_y = dy;
    ev.x = mgr->mouse_state.x;
    ev.y = mgr->mouse_state.y;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_key_event(JiInputManager* mgr, uint32_t key_code,
                                  uint32_t modifiers, bool pressed, uint64_t ts) {
    if (!mgr) return;
    input_ensure_key_capacity(mgr, key_code);
    mgr->key_states[key_code] = pressed;
    mgr->modifiers = modifiers;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_KEY;
    ev.timestamp = ts;
    ev.key_code = key_code;
    ev.modifiers = modifiers;
    ev.pressed = pressed;
    ji_input_dispatch_event(mgr, &ev);
}

/* =========================================================================
 * Touch Integration
 * ========================================================================= */

JI_API JiGestureManager* ji_input_manager_get_gesture_manager(JiInputManager* mgr) {
    return mgr ? mgr->gesture_mgr : NULL;
}

JI_API void ji_input_manager_set_gesture_manager(JiInputManager* mgr, JiGestureManager* gm) {
    if (!mgr) return;
    if (mgr->owns_gesture_mgr && mgr->gesture_mgr) {
        ji_gesture_manager_destroy(mgr->gesture_mgr);
    }
    mgr->gesture_mgr = gm;
    mgr->owns_gesture_mgr = false;
}

/* =========================================================================
 * Pen / Stylus
 * ========================================================================= */

JI_API void ji_input_manager_pen_event(JiInputManager* mgr, float x, float y,
                                  float pressure, float tilt_x, float tilt_y,
                                  bool eraser, bool pressed, uint64_t ts) {
    if (!mgr) return;
    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = pressed ? JI_INPUT_EVENT_PEN_DOWN : JI_INPUT_EVENT_PEN_UP;
    ev.timestamp = ts;
    ev.x = x;
    ev.y = y;
    ev.pressure = pressure;
    ev.tilt_x = tilt_x;
    ev.tilt_y = tilt_y;
    ev.eraser = eraser;
    ev.pressed = pressed;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API bool ji_input_manager_has_pen(const JiInputManager* mgr) {
    if (!mgr) return false;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].type == JI_INPUT_DEVICE_PEN &&
            mgr->devices[i].state != JI_INPUT_DEVICE_STATE_DISCONNECTED) {
            return true;
        }
    }
    return false;
}

/* =========================================================================
 * Gamepad
 * ========================================================================= */

JI_API void ji_input_manager_gamepad_button(JiInputManager* mgr, uint32_t device_id,
                                        uint32_t button, bool pressed, uint64_t ts) {
    if (!mgr) return;
    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_GAMEPAD_BUTTON;
    ev.device_id = device_id;
    ev.timestamp = ts;
    ev.gamepad_button = button;
    ev.pressed = pressed;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_gamepad_axis(JiInputManager* mgr, uint32_t device_id,
                                      int32_t axis, float value, uint64_t ts) {
    if (!mgr) return;
    input_ensure_gamepad_capacity(mgr, device_id);
    GamepadState* gs = &mgr->gamepad_states[device_id];

    switch (axis) {
        case 0: gs->left_x = value; break;
        case 1: gs->left_y = value; break;
        case 2: gs->right_x = value; break;
        case 3: gs->right_y = value; break;
        case 4: gs->left_trigger = value; break;
        case 5: gs->right_trigger = value; break;
        default: break;
    }

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_GAMEPAD_AXIS;
    ev.device_id = device_id;
    ev.timestamp = ts;
    ev.axis_index = axis;
    ev.axis_value = value;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_gamepad_left_stick(const JiInputManager* mgr, uint32_t device_id,
                                           float* x, float* y) {
    if (x) *x = 0;
    if (y) *y = 0;
    if (!mgr || device_id >= mgr->gamepad_state_count) return;
    GamepadState* gs = &mgr->gamepad_states[device_id];
    if (x) *x = gs->left_x;
    if (y) *y = gs->left_y;
}

JI_API void ji_input_manager_gamepad_right_stick(const JiInputManager* mgr, uint32_t device_id,
                                            float* x, float* y) {
    if (x) *x = 0;
    if (y) *y = 0;
    if (!mgr || device_id >= mgr->gamepad_state_count) return;
    GamepadState* gs = &mgr->gamepad_states[device_id];
    if (x) *x = gs->right_x;
    if (y) *y = gs->right_y;
}

JI_API float ji_input_manager_gamepad_trigger(const JiInputManager* mgr, uint32_t device_id,
                                          bool left) {
    if (!mgr || device_id >= mgr->gamepad_state_count) return 0;
    GamepadState* gs = &mgr->gamepad_states[device_id];
    return left ? gs->left_trigger : gs->right_trigger;
}

JI_API void ji_input_manager_gamepad_rumble(JiInputManager* mgr, uint32_t device_id,
                                        float strength, uint32_t duration_ms) {
    (void)mgr;
    (void)device_id;
    (void)strength;
    (void)duration_ms;
    /* Platform-specific rumble would be implemented here */
}

JI_API bool ji_input_manager_has_gamepad(const JiInputManager* mgr) {
    if (!mgr) return false;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].type == JI_INPUT_DEVICE_GAMEPAD &&
            mgr->devices[i].state != JI_INPUT_DEVICE_STATE_DISCONNECTED) {
            return true;
        }
    }
    return false;
}

/* =========================================================================
 * Hot-plug
 * ========================================================================= */

JI_API void ji_input_manager_device_added(JiInputManager* mgr, const JiInputDevice* device) {
    if (!mgr || !device) return;
    input_ensure_device_capacity(mgr);
    mgr->devices[mgr->device_count++] = *device;

    JiInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = JI_INPUT_EVENT_DEVICE_ADDED;
    ev.device = *device;
    ji_input_dispatch_event(mgr, &ev);
}

JI_API void ji_input_manager_device_removed(JiInputManager* mgr, uint32_t device_id) {
    if (!mgr) return;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].id == device_id) {
            JiInputEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.type = JI_INPUT_EVENT_DEVICE_REMOVED;
            ev.device = mgr->devices[i];
            ji_input_dispatch_event(mgr, &ev);

            mgr->devices[i] = mgr->devices[mgr->device_count - 1];
            mgr->device_count--;
            return;
        }
    }
}

/* =========================================================================
 * State Queries
 * ========================================================================= */

JI_API void ji_input_manager_get_mouse_state(const JiInputManager* mgr, JiMouseState* state) {
    if (!mgr || !state) return;
    *state = mgr->mouse_state;
}

JI_API bool ji_input_manager_is_mouse_button_pressed(const JiInputManager* mgr, uint32_t button) {
    if (!mgr) return false;
    return (mgr->mouse_state.buttons & button) != 0;
}

JI_API bool ji_input_manager_is_key_pressed(const JiInputManager* mgr, uint32_t key_code) {
    if (!mgr || key_code >= mgr->key_capacity) return false;
    return mgr->key_states[key_code];
}

JI_API uint32_t ji_input_manager_get_modifiers(const JiInputManager* mgr) {
    return mgr ? mgr->modifiers : 0;
}
