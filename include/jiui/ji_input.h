/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_input.h
 * @brief Advanced input engine — mouse, touch, pen/stylus, gamepad, eye tracking.
 *
 * Provides a unified input API that abstracts over multiple device types.
 * Supports hot-plug detection, device enumeration, and per-device event
 * callbacks. Integrates with the gesture manager for multi-touch.
 */

#ifndef JIUI_INPUT_H
#define JIUI_INPUT_H

#include "ji_types.h"
#include "ji_gesture.h"
#include "ji_platform.h"
#include "ji_api.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Input Device Types
 * ========================================================================= */

typedef enum JiInputDeviceType {
    JI_INPUT_DEVICE_NONE = 0,
    JI_INPUT_DEVICE_MOUSE,
    JI_INPUT_DEVICE_TOUCH,
    JI_INPUT_DEVICE_PEN,
    JI_INPUT_DEVICE_GAMEPAD,
    JI_INPUT_DEVICE_JOYSTICK,
    JI_INPUT_DEVICE_EYE_TRACKER,
    JI_INPUT_DEVICE_HAND_TRACKER
} JiInputDeviceType;

typedef enum JiInputDeviceState {
    JI_INPUT_DEVICE_STATE_DISCONNECTED = 0,
    JI_INPUT_DEVICE_STATE_CONNECTED,
    JI_INPUT_DEVICE_STATE_ACTIVE,
    JI_INPUT_DEVICE_STATE_IDLE
} JiInputDeviceState;

/* =========================================================================
 * Input Device
 * ========================================================================= */

typedef struct JiInputDevice {
    uint32_t            id;
    JiInputDeviceType   type;
    JiInputDeviceState  state;
    char                name[128];
    char                vendor[64];
    uint32_t            button_count;
    uint32_t            axis_count;
    uint32_t            touch_count;   /**< Max simultaneous touches */
    bool                has_pressure;
    bool                has_tilt;
    bool                has_eraser;
} JiInputDevice;

/* =========================================================================
 * Input Event
 * ========================================================================= */

typedef enum JiInputEventType {
    JI_INPUT_EVENT_NONE = 0,
    JI_INPUT_EVENT_MOUSE_MOVE,
    JI_INPUT_EVENT_MOUSE_BUTTON,
    JI_INPUT_EVENT_MOUSE_WHEEL,
    JI_INPUT_EVENT_TOUCH_DOWN,
    JI_INPUT_EVENT_TOUCH_MOVE,
    JI_INPUT_EVENT_TOUCH_UP,
    JI_INPUT_EVENT_TOUCH_CANCEL,
    JI_INPUT_EVENT_PEN_DOWN,
    JI_INPUT_EVENT_PEN_MOVE,
    JI_INPUT_EVENT_PEN_UP,
    JI_INPUT_EVENT_GAMEPAD_BUTTON,
    JI_INPUT_EVENT_GAMEPAD_AXIS,
    JI_INPUT_EVENT_DEVICE_ADDED,
    JI_INPUT_EVENT_DEVICE_REMOVED,
    JI_INPUT_EVENT_KEY
} JiInputEventType;

typedef struct JiInputEvent {
    JiInputEventType  type;
    uint32_t          device_id;
    uint64_t          timestamp;   /**< Milliseconds */

    /* Mouse / Pen position */
    float             x, y;
    float             dx, dy;      /**< Delta */

    /* Mouse buttons */
    uint32_t          button;      /**< JiMouseButton bitmask */
    bool              pressed;

    /* Mouse wheel */
    float             wheel_x;
    float             wheel_y;

    /* Touch / Pen */
    uint32_t          touch_id;
    float             pressure;    /**< 0.0 - 1.0 */
    float             tilt_x;      /**< Pen tilt in degrees */
    float             tilt_y;
    bool              eraser;      /**< Pen eraser mode */

    /* Gamepad */
    uint32_t          gamepad_button;
    float             axis_value;
    int32_t           axis_index;

    /* Keyboard */
    uint32_t          key_code;
    uint32_t          modifiers;

    /* Device change */
    JiInputDevice     device;      /**< For DEVICE_ADDED/REMOVED */
} JiInputEvent;

typedef void (*JiInputCallback)(const JiInputEvent* event, void* user_data);

/* =========================================================================
 * Input Manager
 * ========================================================================= */

typedef struct JiInputManager JiInputManager;

JI_API JiInputManager* ji_input_manager_new(void);
JI_API void            ji_input_manager_destroy(JiInputManager* mgr);

/** Set a global input callback for all events. */
JI_API void ji_input_manager_set_callback(JiInputManager* mgr,
                                     JiInputCallback callback,
                                     void* user_data);

/** Set a callback for a specific device type. */
JI_API void ji_input_manager_set_device_callback(JiInputManager* mgr,
                                            JiInputDeviceType type,
                                            JiInputCallback callback,
                                            void* user_data);

/* =========================================================================
 * Device Enumeration
 * ========================================================================= */

/** Get the number of connected devices. */
JI_API uint32_t ji_input_manager_device_count(const JiInputManager* mgr);

/** Get device info by index. */
JI_API bool ji_input_manager_get_device(const JiInputManager* mgr, uint32_t index,
                                   JiInputDevice* out_device);

/** Get device info by ID. */
JI_API bool ji_input_manager_get_device_by_id(const JiInputManager* mgr, uint32_t id,
                                        JiInputDevice* out_device);

/** Find devices by type. Returns count found. */
JI_API uint32_t ji_input_manager_find_devices(const JiInputManager* mgr,
                                          JiInputDeviceType type,
                                          JiInputDevice* out_devices,
                                          uint32_t max_count);

/* =========================================================================
 * Event Dispatch (platform feeds events in)
 * ========================================================================= */

JI_API void ji_input_manager_dispatch_event(JiInputManager* mgr, const JiInputEvent* event);

/** Dispatch a mouse move event. */
JI_API void ji_input_manager_mouse_move(JiInputManager* mgr, float x, float y, uint64_t ts);

/** Dispatch a mouse button event. */
JI_API void ji_input_manager_mouse_button(JiInputManager* mgr, uint32_t button,
                                     bool pressed, uint64_t ts);

/** Dispatch a mouse wheel event. */
JI_API void ji_input_manager_mouse_wheel(JiInputManager* mgr, float dx, float dy, uint64_t ts);

/** Dispatch a keyboard event. */
JI_API void ji_input_manager_key_event(JiInputManager* mgr, uint32_t key_code,
                                  uint32_t modifiers, bool pressed, uint64_t ts);

/* =========================================================================
 * Touch Integration
 * ========================================================================= */

/** Get the gesture manager associated with this input manager. */
JI_API JiGestureManager* ji_input_manager_get_gesture_manager(JiInputManager* mgr);

/** Set a custom gesture manager. */
JI_API void ji_input_manager_set_gesture_manager(JiInputManager* mgr, JiGestureManager* gm);

/* =========================================================================
 * Pen / Stylus
 * ========================================================================= */

/** Dispatch a pen event. */
JI_API void ji_input_manager_pen_event(JiInputManager* mgr, float x, float y,
                                  float pressure, float tilt_x, float tilt_y,
                                  bool eraser, bool pressed, uint64_t ts);

/** Check if any pen device is connected. */
JI_API bool ji_input_manager_has_pen(const JiInputManager* mgr);

/* =========================================================================
 * Gamepad / Joystick
 * ========================================================================= */

/** Dispatch a gamepad button event. */
JI_API void ji_input_manager_gamepad_button(JiInputManager* mgr, uint32_t device_id,
                                        uint32_t button, bool pressed, uint64_t ts);

/** Dispatch a gamepad axis event. */
JI_API void ji_input_manager_gamepad_axis(JiInputManager* mgr, uint32_t device_id,
                                      int32_t axis, float value, uint64_t ts);

/** Get the left stick values (-1.0 to 1.0). */
JI_API void ji_input_manager_gamepad_left_stick(const JiInputManager* mgr, uint32_t device_id,
                                           float* x, float* y);

/** Get the right stick values (-1.0 to 1.0). */
JI_API void ji_input_manager_gamepad_right_stick(const JiInputManager* mgr, uint32_t device_id,
                                            float* x, float* y);

/** Get trigger values (0.0 to 1.0). */
JI_API float ji_input_manager_gamepad_trigger(const JiInputManager* mgr, uint32_t device_id,
                                          bool left);

/** Set rumble/vibration. strength 0.0-1.0, duration_ms. */
JI_API void ji_input_manager_gamepad_rumble(JiInputManager* mgr, uint32_t device_id,
                                        float strength, uint32_t duration_ms);

/** Check if any gamepad is connected. */
JI_API bool ji_input_manager_has_gamepad(const JiInputManager* mgr);

/* =========================================================================
 * Hot-plug
 * ========================================================================= */

/** Notify the manager that a device was connected. */
JI_API void ji_input_manager_device_added(JiInputManager* mgr, const JiInputDevice* device);

/** Notify the manager that a device was removed. */
JI_API void ji_input_manager_device_removed(JiInputManager* mgr, uint32_t device_id);

/* =========================================================================
 * Mouse State Query
 * ========================================================================= */

typedef struct JiMouseState {
    float    x, y;
    uint32_t buttons;  /**< Bitmask of currently pressed buttons */
    float    wheel_delta_x;
    float    wheel_delta_y;
} JiMouseState;

JI_API void ji_input_manager_get_mouse_state(const JiInputManager* mgr, JiMouseState* state);

/** Check if a mouse button is currently pressed. */
JI_API bool ji_input_manager_is_mouse_button_pressed(const JiInputManager* mgr, uint32_t button);

/** Check if a key is currently pressed. */
JI_API bool ji_input_manager_is_key_pressed(const JiInputManager* mgr, uint32_t key_code);

JI_API uint32_t ji_input_manager_get_modifiers(const JiInputManager* mgr);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_INPUT_H */
