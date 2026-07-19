/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_gesture.h
 * @brief Gesture recognition framework — swipe, pinch, rotate, drag, custom.
 *
 * Provides a multi-touch gesture recognition system that can detect
 * swipes, pinches, rotations, long-press, tap, double-tap, and custom
 * recorded gestures. Supports gesture conflict resolution via priority
 * and simultaneous recognition.
 */

#ifndef JIUI_GESTURE_H
#define JIUI_GESTURE_H

#include "ji_types.h"
#include "ji_api.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Gesture Types
 * ========================================================================= */

typedef enum JiGestureType {
    JI_GESTURE_NONE = 0,
    JI_GESTURE_TAP,
    JI_GESTURE_DOUBLE_TAP,
    JI_GESTURE_LONG_PRESS,
    JI_GESTURE_SWIPE,
    JI_GESTURE_PINCH,
    JI_GESTURE_ROTATE,
    JI_GESTURE_DRAG,
    JI_GESTURE_PAN,
    JI_GESTURE_CUSTOM
} JiGestureType;

typedef enum JiGestureState {
    JI_GESTURE_STATE_IDLE,
    JI_GESTURE_STATE_POSSIBLE,
    JI_GESTURE_STATE_BEGAN,
    JI_GESTURE_STATE_CHANGED,
    JI_GESTURE_STATE_ENDED,
    JI_GESTURE_STATE_CANCELLED,
    JI_GESTURE_STATE_FAILED
} JiGestureState;

typedef enum JiSwipeDirection {
    JI_SWIPE_NONE  = 0,
    JI_SWIPE_LEFT  = 1,
    JI_SWIPE_RIGHT = 2,
    JI_SWIPE_UP    = 4,
    JI_SWIPE_DOWN  = 8
} JiSwipeDirection;

/* =========================================================================
 * Gesture Event
 * ========================================================================= */

typedef struct JiGestureEvent {
    JiGestureType   type;
    JiGestureState   state;
    float            x, y;          /**< Center point */
    float            dx, dy;        /**< Delta since last event */
    float            velocity_x;    /**< px/sec */
    float            velocity_y;    /**< px/sec */
    float            scale;         /**< Pinch scale factor */
    float            rotation;      /**< Rotation in radians */
    JiSwipeDirection direction;     /**< Swipe direction */
    uint32_t         touch_count;   /**< Number of touches */
    uint64_t         timestamp;     /**< Milliseconds */
    void*            user_data;      /**< Custom data */
} JiGestureEvent;

typedef void (*JiGestureCallback)(const JiGestureEvent* event, void* user_data);

/* =========================================================================
 * Gesture Recognizer
 * ========================================================================= */

typedef struct JiGestureRecognizer JiGestureRecognizer;

JI_API JiGestureRecognizer* ji_gesture_recognizer_new(JiGestureType type);
JI_API void                 ji_gesture_recognizer_destroy(JiGestureRecognizer* rec);

JI_API JiGestureType        ji_gesture_recognizer_get_type(const JiGestureRecognizer* rec);
JI_API JiGestureState       ji_gesture_recognizer_get_state(const JiGestureRecognizer* rec);

/** Set the callback invoked when the gesture state changes. */
JI_API void ji_gesture_recognizer_set_callback(JiGestureRecognizer* rec,
                                          JiGestureCallback callback,
                                          void* user_data);

/** Set the priority for conflict resolution (higher wins). */
JI_API void ji_gesture_recognizer_set_priority(JiGestureRecognizer* rec, int32_t priority);
JI_API int32_t ji_gesture_recognizer_get_priority(const JiGestureRecognizer* rec);

/** Allow simultaneous recognition with other gestures. */
JI_API void ji_gesture_recognizer_set_simultaneous(JiGestureRecognizer* rec, bool allow);
JI_API bool ji_gesture_recognizer_get_simultaneous(const JiGestureRecognizer* rec);

/* Configuration — Tap */
JI_API void ji_gesture_tap_set_max_duration(JiGestureRecognizer* rec, uint64_t ms);
JI_API void ji_gesture_tap_set_max_movement(JiGestureRecognizer* rec, float pixels);

/* Configuration — Long Press */
JI_API void ji_gesture_long_press_set_min_duration(JiGestureRecognizer* rec, uint64_t ms);
JI_API void ji_gesture_long_press_set_max_movement(JiGestureRecognizer* rec, float pixels);

/* Configuration — Swipe */
JI_API void ji_gesture_swipe_set_directions(JiGestureRecognizer* rec, uint32_t dir_mask);
JI_API void ji_gesture_swipe_set_min_distance(JiGestureRecognizer* rec, float pixels);
JI_API void ji_gesture_swipe_set_min_velocity(JiGestureRecognizer* rec, float px_per_sec);

/* Configuration — Pinch */
JI_API void ji_gesture_pinch_set_min_scale(JiGestureRecognizer* rec, float scale);

/* Configuration — Rotate */
JI_API void ji_gesture_rotate_set_min_angle(JiGestureRecognizer* rec, float radians);

/* Configuration — Drag / Pan */
JI_API void ji_gesture_drag_set_min_distance(JiGestureRecognizer* rec, float pixels);
JI_API void ji_gesture_drag_set_inertia(JiGestureRecognizer* rec, bool enabled);
JI_API void ji_gesture_drag_set_friction(JiGestureRecognizer* rec, float friction);

/* =========================================================================
 * Gesture Manager
 * ========================================================================= */

typedef struct JiGestureManager JiGestureManager;

JI_API JiGestureManager* ji_gesture_manager_new(void);
JI_API void              ji_gesture_manager_destroy(JiGestureManager* mgr);

/** Add a gesture recognizer. */
JI_API void ji_gesture_manager_add(JiGestureManager* mgr, JiGestureRecognizer* rec);

/** Remove a gesture recognizer. */
JI_API void ji_gesture_manager_remove(JiGestureManager* mgr, JiGestureRecognizer* rec);

/** Remove all recognizers. */
JI_API void ji_gesture_manager_clear(JiGestureManager* mgr);

/** Get the number of registered recognizers. */
JI_API uint32_t ji_gesture_manager_count(const JiGestureManager* mgr);

/* =========================================================================
 * Touch Input (fed into the gesture manager)
 * ========================================================================= */

typedef struct JiTouchPoint {
    uint32_t id;       /**< Unique touch ID */
    float    x, y;     /**< Position */
    float    pressure; /**< 0.0 - 1.0 */
    uint64_t timestamp;/**< Milliseconds */
} JiTouchPoint;

/** Feed a touch-down event. */
JI_API void ji_gesture_manager_touch_down(JiGestureManager* mgr, const JiTouchPoint* point);

/** Feed a touch-move event. */
JI_API void ji_gesture_manager_touch_move(JiGestureManager* mgr, const JiTouchPoint* point);

/** Feed a touch-up event. */
JI_API void ji_gesture_manager_touch_up(JiGestureManager* mgr, uint32_t id, uint64_t timestamp);

/** Cancel all active touches. */
JI_API void ji_gesture_manager_touch_cancel(JiGestureManager* mgr, uint64_t timestamp);

/** Update inertia / physics (call per frame). dt in seconds. */
JI_API void ji_gesture_manager_update(JiGestureManager* mgr, float dt);

/* =========================================================================
 * Custom Gesture Recording & Matching
 * ========================================================================= */

typedef struct JiGestureTemplate JiGestureTemplate;

/** Start recording a new gesture template. */
JI_API JiGestureTemplate* ji_gesture_template_new(const char* name);
JI_API void                ji_gesture_template_destroy(JiGestureTemplate* tmpl);

/** Add a point to the template during recording. */
JI_API void ji_gesture_template_add_point(JiGestureTemplate* tmpl, float x, float y, uint64_t ts);

/** Get the number of points in the template. */
JI_API uint32_t ji_gesture_template_point_count(const JiGestureTemplate* tmpl);

/** Match a recorded path against the template. Returns confidence 0..1. */
JI_API float ji_gesture_template_match(const JiGestureTemplate* tmpl,
                                  const JiTouchPoint* points, uint32_t count);

/** Create a custom gesture recognizer from a template. */
JI_API JiGestureRecognizer* ji_gesture_custom_new(const JiGestureTemplate* tmpl,
                                             float min_confidence);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_GESTURE_H */
