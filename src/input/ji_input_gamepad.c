/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input_gamepad.c
 * @brief Gamepad/joystick input handling — buttons, axes, triggers, rumble.
 */

#include "jiui/ji_api.h"
#include "ji_input_internal.h"
#include "jiui/ji_memory.h"
#include <string.h>

/* Standard gamepad button mapping */
typedef enum JiGamepadButton {
    JI_GAMEPAD_A          = 0,
    JI_GAMEPAD_B          = 1,
    JI_GAMEPAD_X          = 2,
    JI_GAMEPAD_Y          = 3,
    JI_GAMEPAD_LB         = 4,
    JI_GAMEPAD_RB         = 5,
    JI_GAMEPAD_BACK       = 6,
    JI_GAMEPAD_START      = 7,
    JI_GAMEPAD_GUIDE      = 8,
    JI_GAMEPAD_LSTICK     = 9,
    JI_GAMEPAD_RSTICK     = 10,
    JI_GAMEPAD_DPAD_UP    = 11,
    JI_GAMEPAD_DPAD_DOWN  = 12,
    JI_GAMEPAD_DPAD_LEFT  = 13,
    JI_GAMEPAD_DPAD_RIGHT = 14
} JiGamepadButton;

JI_API void ji_input_manager_gamepad_connect(JiInputManager* mgr, uint32_t device_id,
                                          const char* name) {
    if (!mgr) return;
    JiInputDevice dev;
    memset(&dev, 0, sizeof(dev));
    dev.id = device_id;
    dev.type = JI_INPUT_DEVICE_GAMEPAD;
    dev.state = JI_INPUT_DEVICE_STATE_CONNECTED;
    dev.button_count = 15;
    dev.axis_count = 6;
    if (name) {
        strncpy(dev.name, name, sizeof(dev.name) - 1);
    }
    ji_input_manager_device_added(mgr, &dev);
}

JI_API void ji_input_manager_gamepad_disconnect(JiInputManager* mgr, uint32_t device_id) {
    if (!mgr) return;
    ji_input_manager_device_removed(mgr, device_id);
}

JI_API void ji_input_manager_gamepad_dpad(JiInputManager* mgr, uint32_t device_id,
                                       int32_t dx, int32_t dy, uint64_t ts) {
    if (!mgr) return;
    /* DPad is represented as 4 buttons */
    if (dx < 0) {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_LEFT, true, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_RIGHT, false, ts);
    } else if (dx > 0) {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_RIGHT, true, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_LEFT, false, ts);
    } else {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_LEFT, false, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_RIGHT, false, ts);
    }

    if (dy < 0) {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_UP, true, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_DOWN, false, ts);
    } else if (dy > 0) {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_DOWN, true, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_UP, false, ts);
    } else {
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_UP, false, ts);
        ji_input_manager_gamepad_button(mgr, device_id, JI_GAMEPAD_DPAD_DOWN, false, ts);
    }
}

JI_API void ji_input_manager_gamepad_set_deadzone(JiInputManager* mgr, uint32_t device_id,
                                               float deadzone) {
    (void)mgr;
    (void)device_id;
    (void)deadzone;
    /* Deadzone would be applied to axis values in a real implementation */
}

JI_API float ji_input_manager_gamepad_apply_deadzone(float value, float deadzone) {
    if (value > deadzone) return (value - deadzone) / (1.0f - deadzone);
    if (value < -deadzone) return (value + deadzone) / (1.0f - deadzone);
    return 0.0f;
}

JI_API uint32_t ji_input_manager_gamepad_count(const JiInputManager* mgr) {
    if (!mgr) return 0;
    uint32_t count = 0;
    for (uint32_t i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].type == JI_INPUT_DEVICE_GAMEPAD &&
            mgr->devices[i].state != JI_INPUT_DEVICE_STATE_DISCONNECTED) {
            count++;
        }
    }
    return count;
}
