/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input_internal.h
 * @brief Internal input manager structure — shared across input source files.
 */

#ifndef JIUI_INPUT_INTERNAL_H
#define JIUI_INPUT_INTERNAL_H

#include "jiui/ji_input.h"
#include "jiui/ji_gesture.h"
#include <stdbool.h>

typedef struct GamepadState {
    float left_x, left_y;
    float right_x, right_y;
    float left_trigger, right_trigger;
} GamepadState;

struct JiInputManager {
    JiInputCallback     global_callback;
    void*               global_user_data;

    JiInputCallback     device_callbacks[9];
    void*               device_user_data[9];

    JiInputDevice*      devices;
    uint32_t            device_count;
    uint32_t            device_capacity;

    JiMouseState        mouse_state;
    uint32_t            modifiers;
    bool*               key_states;
    uint32_t            key_capacity;

    JiGestureManager*   gesture_mgr;
    bool                owns_gesture_mgr;

    GamepadState*       gamepad_states;
    uint32_t            gamepad_state_count;
    uint32_t            gamepad_state_capacity;
};

/** Internal dispatch function (not part of public API). */
void ji_input_dispatch_event(JiInputManager* mgr, const JiInputEvent* event);

#endif /* JIUI_INPUT_INTERNAL_H */
