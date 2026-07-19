/**
 * JiUI - Animation Framework header
 * Provides property animation, easing curves, and animation groups.
 * Surpasses Qt6 with built-in spring physics and bezier easing.
 */

#ifndef JIUI_ANIMATION_H
#define JIUI_ANIMATION_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Easing Curves — 40+ presets, custom cubic bezier, spring physics
 * ========================================================================= */
typedef enum JiEasingType {
    /* Linear */
    JI_EASE_LINEAR = 0,
    /* Quadratic */
    JI_EASE_IN_QUAD, JI_EASE_OUT_QUAD, JI_EASE_IN_OUT_QUAD,
    /* Cubic */
    JI_EASE_IN_CUBIC, JI_EASE_OUT_CUBIC, JI_EASE_IN_OUT_CUBIC,
    /* Quartic */
    JI_EASE_IN_QUART, JI_EASE_OUT_QUART, JI_EASE_IN_OUT_QUART,
    /* Quintic */
    JI_EASE_IN_QUINT, JI_EASE_OUT_QUINT, JI_EASE_IN_OUT_QUINT,
    /* Sinusoidal */
    JI_EASE_IN_SINE, JI_EASE_OUT_SINE, JI_EASE_IN_OUT_SINE,
    /* Exponential */
    JI_EASE_IN_EXPO, JI_EASE_OUT_EXPO, JI_EASE_IN_OUT_EXPO,
    /* Circular */
    JI_EASE_IN_CIRC, JI_EASE_OUT_CIRC, JI_EASE_IN_OUT_CIRC,
    /* Back */
    JI_EASE_IN_BACK, JI_EASE_OUT_BACK, JI_EASE_IN_OUT_BACK,
    /* Elastic */
    JI_EASE_IN_ELASTIC, JI_EASE_OUT_ELASTIC, JI_EASE_IN_OUT_ELASTIC,
    /* Bounce */
    JI_EASE_IN_BOUNCE, JI_EASE_OUT_BOUNCE, JI_EASE_IN_OUT_BOUNCE,
    /* Custom */
    JI_EASE_CUBIC_BEZIER,   /* uses control points */
    JI_EASE_SPRING           /* spring physics (beyond Qt6) */
} JiEasingType;

typedef struct JiEasingCurve {
    JiEasingType type;
    /* Cubic bezier control points (used when type == JI_EASE_CUBIC_BEZIER) */
    double cp1x, cp1y, cp2x, cp2y;
    /* Spring parameters (used when type == JI_EASE_SPRING) */
    double spring_stiffness;   /* default 200 */
    double spring_damping;     /* default 20 */
    double spring_mass;        /* default 1 */
} JiEasingCurve;

JI_API JiEasingCurve ji_easing_curve_new(JiEasingType type);
JI_API void ji_easing_curve_set_bezier(JiEasingCurve* curve, double cp1x, double cp1y, double cp2x, double cp2y);
JI_API void ji_easing_curve_set_spring(JiEasingCurve* curve, double stiffness, double damping, double mass);
JI_API double ji_easing_curve_value(const JiEasingCurve* curve, double progress);

/* =========================================================================
 * Animation — base animation with duration, loop count, direction
 * ========================================================================= */
typedef enum JiAnimationState {
    JI_ANIMATION_STOPPED = 0,
    JI_ANIMATION_PAUSED = 1,
    JI_ANIMATION_RUNNING = 2
} JiAnimationState;

typedef enum JiAnimationDirection {
    JI_ANIMATION_FORWARD = 0,
    JI_ANIMATION_REVERSE = 1,
    JI_ANIMATION_ALTERNATE = 2,
    JI_ANIMATION_ALTERNATE_REVERSE = 3
} JiAnimationDirection;

typedef struct JiAnimation {
    JiAnimationState      state;
    int                   duration_ms;      /* 0 = instant, -1 = infinite */
    int                   current_loop;
    int                   loop_count;        /* -1 = infinite */
    JiAnimationDirection  direction;
    JiEasingCurve         easing;
    double                current_progress;  /* 0.0 - 1.0 */
    uint64_t              start_time_ms;
    uint64_t              pause_time_ms;
    void (*on_finished)(struct JiAnimation* anim, void* user_data);
    void*                 user_data;
} JiAnimation;

JI_API void ji_animation_init(JiAnimation* anim);
JI_API void ji_animation_set_duration(JiAnimation* anim, int ms);
JI_API void ji_animation_set_loop_count(JiAnimation* anim, int count);
JI_API void ji_animation_set_direction(JiAnimation* anim, JiAnimationDirection dir);
JI_API void ji_animation_set_easing(JiAnimation* anim, JiEasingCurve easing);
JI_API void ji_animation_start(JiAnimation* anim);
JI_API void ji_animation_pause(JiAnimation* anim);
JI_API void ji_animation_resume(JiAnimation* anim);
JI_API void ji_animation_stop(JiAnimation* anim);
JI_API double ji_animation_current_value(const JiAnimation* anim);
JI_API bool ji_animation_update(JiAnimation* anim, uint64_t current_time_ms);

/* =========================================================================
 * Property Animation — animates a double value from start to end
 * ========================================================================= */
typedef struct JiPropertyAnimation {
    JiAnimation  base;
    double       start_value;
    double       end_value;
    double       current_value;
    void (*on_value_changed)(struct JiPropertyAnimation* anim, double value, void* user_data);
} JiPropertyAnimation;

JI_API JiPropertyAnimation* ji_property_animation_new(void);
JI_API void ji_property_animation_destroy(JiPropertyAnimation* anim);
JI_API void ji_property_animation_set_range(JiPropertyAnimation* anim, double start, double end);

/* =========================================================================
 * Animation Group — parallel or sequential animation groups
 * ========================================================================= */
typedef struct JiAnimationGroup JiAnimationGroup;

typedef enum JiAnimationGroupMode {
    JI_ANIMATION_GROUP_PARALLEL = 0,
    JI_ANIMATION_GROUP_SEQUENTIAL = 1
} JiAnimationGroupMode;

typedef struct JiAnimationGroup {
    JiAnimation          base;
    JiAnimation**        children;
    int                  child_count;
    int                  child_capacity;
    JiAnimationGroupMode mode;
} JiAnimationGroup;

JI_API JiAnimationGroup* ji_animation_group_new(JiAnimationGroupMode mode);
JI_API void ji_animation_group_destroy(JiAnimationGroup* group);
JI_API void ji_animation_group_add(JiAnimationGroup* group, JiAnimation* anim);
JI_API void ji_animation_group_remove(JiAnimationGroup* group, JiAnimation* anim);

/* =========================================================================
 * Animation Driver — drives all active animations
 * ========================================================================= */
typedef struct JiAnimationDriver {
    JiAnimation**  animations;
    int            animation_count;
    int            animation_capacity;
} JiAnimationDriver;

JI_API JiAnimationDriver* ji_animation_driver_new(void);
JI_API void ji_animation_driver_destroy(JiAnimationDriver* driver);
JI_API void ji_animation_driver_register(JiAnimationDriver* driver, JiAnimation* anim);
JI_API void ji_animation_driver_unregister(JiAnimationDriver* driver, JiAnimation* anim);
JI_API void ji_animation_driver_tick(JiAnimationDriver* driver, uint64_t current_time_ms);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_ANIMATION_H */
