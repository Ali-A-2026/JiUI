/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_gesture.c
 * @brief Gesture recognition implementation.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_gesture.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================================
 * Gesture Recognizer
 * ========================================================================= */

typedef struct GestureConfig {
    /* Tap */
    uint64_t tap_max_duration;   /* ms */
    float    tap_max_movement;   /* px */

    /* Long press */
    uint64_t long_press_min_duration;
    float    long_press_max_movement;

    /* Swipe */
    uint32_t swipe_directions;   /* bitmask of JiSwipeDirection */
    float    swipe_min_distance;
    float    swipe_min_velocity;

    /* Pinch */
    float    pinch_min_scale;

    /* Rotate */
    float    rotate_min_angle;

    /* Drag / Pan */
    float    drag_min_distance;
    bool     drag_inertia;
    float    drag_friction;
} GestureConfig;

struct JiGestureRecognizer {
    JiGestureType      type;
    JiGestureState     state;
    JiGestureCallback  callback;
    void*              user_data;
    int32_t            priority;
    bool               simultaneous;
    GestureConfig      config;

    /* Tracking state */
    JiTouchPoint*      touches;
    uint32_t           touch_count;
    uint32_t           touch_capacity;
    float              start_x, start_y;
    float              last_x, last_y;
    uint64_t           start_time;
    float              initial_distance;  /* For pinch */
    float              initial_angle;     /* For rotate */
    float              velocity_x, velocity_y;

    /* Custom gesture */
    const JiGestureTemplate* template;
    float                    min_confidence;
};

static void gesture_recognizer_ensure_touch_capacity(JiGestureRecognizer* rec, uint32_t n) {
    if (n > rec->touch_capacity) {
        rec->touch_capacity = rec->touch_capacity ? rec->touch_capacity * 2 : 4;
        while (rec->touch_capacity < n) rec->touch_capacity *= 2;
        rec->touches = ji_realloc(rec->touches, rec->touch_capacity * sizeof(JiTouchPoint));
    }
}

static void gesture_recognizer_reset(JiGestureRecognizer* rec) {
    rec->state = JI_GESTURE_STATE_IDLE;
    rec->touch_count = 0;
    rec->velocity_x = 0;
    rec->velocity_y = 0;
}

static void gesture_recognizer_fire(JiGestureRecognizer* rec, JiGestureState state,
                                        float x, float y, float dx, float dy) {
    if (!rec->callback) return;
    JiGestureEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = rec->type;
    ev.state = state;
    ev.x = x;
    ev.y = y;
    ev.dx = dx;
    ev.dy = dy;
    ev.velocity_x = rec->velocity_x;
    ev.velocity_y = rec->velocity_y;
    ev.touch_count = rec->touch_count;
    ev.user_data = rec->user_data;
    rec->callback(&ev, rec->user_data);
    rec->state = state;
}

static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static float angle_between(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

static JiSwipeDirection detect_swipe_direction(float dx, float dy) {
    if (fabsf(dx) > fabsf(dy)) {
        return dx > 0 ? JI_SWIPE_RIGHT : JI_SWIPE_LEFT;
    } else {
        return dy > 0 ? JI_SWIPE_DOWN : JI_SWIPE_UP;
    }
}

JI_API JiGestureRecognizer* ji_gesture_recognizer_new(JiGestureType type) {
    JiGestureRecognizer* rec = ji_calloc(1, sizeof(JiGestureRecognizer));
    if (!rec) return NULL;
    rec->type = type;
    rec->state = JI_GESTURE_STATE_IDLE;
    rec->priority = 0;
    rec->simultaneous = false;

    /* Default config */
    rec->config.tap_max_duration = 300;
    rec->config.tap_max_movement = 10.0f;
    rec->config.long_press_min_duration = 500;
    rec->config.long_press_max_movement = 10.0f;
    rec->config.swipe_directions = JI_SWIPE_LEFT | JI_SWIPE_RIGHT | JI_SWIPE_UP | JI_SWIPE_DOWN;
    rec->config.swipe_min_distance = 50.0f;
    rec->config.swipe_min_velocity = 100.0f;
    rec->config.pinch_min_scale = 0.1f;
    rec->config.rotate_min_angle = 0.1f;
    rec->config.drag_min_distance = 10.0f;
    rec->config.drag_inertia = false;
    rec->config.drag_friction = 0.95f;

    return rec;
}

JI_API void ji_gesture_recognizer_destroy(JiGestureRecognizer* rec) {
    if (!rec) return;
    ji_free(rec->touches);
    ji_free(rec);
}

JI_API JiGestureType ji_gesture_recognizer_get_type(const JiGestureRecognizer* rec) {
    return rec ? rec->type : JI_GESTURE_NONE;
}

JI_API JiGestureState ji_gesture_recognizer_get_state(const JiGestureRecognizer* rec) {
    return rec ? rec->state : JI_GESTURE_STATE_IDLE;
}

JI_API void ji_gesture_recognizer_set_callback(JiGestureRecognizer* rec,
                                          JiGestureCallback callback,
                                          void* user_data) {
    if (rec) {
        rec->callback = callback;
        rec->user_data = user_data;
    }
}

JI_API void ji_gesture_recognizer_set_priority(JiGestureRecognizer* rec, int32_t priority) {
    if (rec) rec->priority = priority;
}

int32_t JI_API ji_gesture_recognizer_get_priority(const JiGestureRecognizer* rec) {
    return rec ? rec->priority : 0;
}

JI_API void ji_gesture_recognizer_set_simultaneous(JiGestureRecognizer* rec, bool allow) {
    if (rec) rec->simultaneous = allow;
}

JI_API bool ji_gesture_recognizer_get_simultaneous(const JiGestureRecognizer* rec) {
    return rec ? rec->simultaneous : false;
}

/* Config setters */
JI_API void ji_gesture_tap_set_max_duration(JiGestureRecognizer* rec, uint64_t ms) {
    if (rec) rec->config.tap_max_duration = ms;
}
JI_API void ji_gesture_tap_set_max_movement(JiGestureRecognizer* rec, float pixels) {
    if (rec) rec->config.tap_max_movement = pixels;
}
JI_API void ji_gesture_long_press_set_min_duration(JiGestureRecognizer* rec, uint64_t ms) {
    if (rec) rec->config.long_press_min_duration = ms;
}
JI_API void ji_gesture_long_press_set_max_movement(JiGestureRecognizer* rec, float pixels) {
    if (rec) rec->config.long_press_max_movement = pixels;
}
JI_API void ji_gesture_swipe_set_directions(JiGestureRecognizer* rec, uint32_t dir_mask) {
    if (rec) rec->config.swipe_directions = dir_mask;
}
JI_API void ji_gesture_swipe_set_min_distance(JiGestureRecognizer* rec, float pixels) {
    if (rec) rec->config.swipe_min_distance = pixels;
}
JI_API void ji_gesture_swipe_set_min_velocity(JiGestureRecognizer* rec, float px_per_sec) {
    if (rec) rec->config.swipe_min_velocity = px_per_sec;
}
JI_API void ji_gesture_pinch_set_min_scale(JiGestureRecognizer* rec, float scale) {
    if (rec) rec->config.pinch_min_scale = scale;
}
JI_API void ji_gesture_rotate_set_min_angle(JiGestureRecognizer* rec, float radians) {
    if (rec) rec->config.rotate_min_angle = radians;
}
JI_API void ji_gesture_drag_set_min_distance(JiGestureRecognizer* rec, float pixels) {
    if (rec) rec->config.drag_min_distance = pixels;
}
JI_API void ji_gesture_drag_set_inertia(JiGestureRecognizer* rec, bool enabled) {
    if (rec) rec->config.drag_inertia = enabled;
}
JI_API void ji_gesture_drag_set_friction(JiGestureRecognizer* rec, float friction) {
    if (rec) rec->config.drag_friction = friction;
}

/* =========================================================================
 * Touch Processing
 * ========================================================================= */

static void process_touch_down(JiGestureRecognizer* rec, const JiTouchPoint* p) {
    gesture_recognizer_ensure_touch_capacity(rec, rec->touch_count + 1);
    rec->touches[rec->touch_count++] = *p;

    if (rec->touch_count == 1) {
        rec->start_x = p->x;
        rec->start_y = p->y;
        rec->last_x = p->x;
        rec->last_y = p->y;
        rec->start_time = p->timestamp;
        rec->state = JI_GESTURE_STATE_POSSIBLE;
    }

    if (rec->touch_count == 2) {
        rec->initial_distance = distance(rec->touches[0].x, rec->touches[0].y,
                                           rec->touches[1].x, rec->touches[1].y);
        rec->initial_angle = angle_between(rec->touches[0].x, rec->touches[0].y,
                                             rec->touches[1].x, rec->touches[1].y);
    }
}

static void process_touch_move(JiGestureRecognizer* rec, const JiTouchPoint* p) {
    /* Update the touch point */
    for (uint32_t i = 0; i < rec->touch_count; i++) {
        if (rec->touches[i].id == p->id) {
            rec->touches[i] = *p;
            break;
        }
    }

    if (rec->touch_count == 0) return;

    float cx = 0, cy = 0;
    for (uint32_t i = 0; i < rec->touch_count; i++) {
        cx += rec->touches[i].x;
        cy += rec->touches[i].y;
    }
    cx /= rec->touch_count;
    cy /= rec->touch_count;

    float dx = cx - rec->last_x;
    float dy = cy - rec->last_y;
    float dt = (p->timestamp - rec->start_time) / 1000.0f;
    if (dt > 0.001f) {
        rec->velocity_x = dx / dt;
        rec->velocity_y = dy / dt;
    }

    float total_dist = distance(rec->start_x, rec->start_y, cx, cy);

    switch (rec->type) {
        case JI_GESTURE_TAP:
            if (total_dist > rec->config.tap_max_movement) {
                rec->state = JI_GESTURE_STATE_FAILED;
            }
            break;

        case JI_GESTURE_LONG_PRESS:
            if (total_dist > rec->config.long_press_max_movement) {
                rec->state = JI_GESTURE_STATE_FAILED;
            } else if ((p->timestamp - rec->start_time) >= rec->config.long_press_min_duration &&
                       rec->state == JI_GESTURE_STATE_POSSIBLE) {
                gesture_recognizer_fire(rec, JI_GESTURE_STATE_BEGAN, cx, cy, 0, 0);
            }
            break;

        case JI_GESTURE_SWIPE: {
            if (total_dist >= rec->config.swipe_min_distance) {
                JiSwipeDirection dir = detect_swipe_direction(cx - rec->start_x, cy - rec->start_y);
                if (rec->config.swipe_directions & dir) {
                    JiGestureEvent ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.type = rec->type;
                    ev.state = JI_GESTURE_STATE_ENDED;
                    ev.x = cx;
                    ev.y = cy;
                    ev.dx = cx - rec->start_x;
                    ev.dy = cy - rec->start_y;
                    ev.velocity_x = rec->velocity_x;
                    ev.velocity_y = rec->velocity_y;
                    ev.direction = dir;
                    ev.touch_count = rec->touch_count;
                    ev.timestamp = p->timestamp;
                    ev.user_data = rec->user_data;
                    if (rec->callback) rec->callback(&ev, rec->user_data);
                    rec->state = JI_GESTURE_STATE_ENDED;
                }
            }
            break;
        }

        case JI_GESTURE_DRAG:
        case JI_GESTURE_PAN: {
            if (rec->state == JI_GESTURE_STATE_POSSIBLE && total_dist >= rec->config.drag_min_distance) {
                gesture_recognizer_fire(rec, JI_GESTURE_STATE_BEGAN, cx, cy, dx, dy);
            } else if (rec->state == JI_GESTURE_STATE_BEGAN || rec->state == JI_GESTURE_STATE_CHANGED) {
                gesture_recognizer_fire(rec, JI_GESTURE_STATE_CHANGED, cx, cy, dx, dy);
            }
            break;
        }

        case JI_GESTURE_PINCH: {
            if (rec->touch_count >= 2) {
                float d = distance(rec->touches[0].x, rec->touches[0].y,
                                     rec->touches[1].x, rec->touches[1].y);
                float scale = rec->initial_distance > 0 ? d / rec->initial_distance : 1.0f;
                if (fabsf(scale - 1.0f) >= rec->config.pinch_min_scale) {
                    JiGestureEvent ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.type = rec->type;
                    ev.state = (rec->state == JI_GESTURE_STATE_POSSIBLE) ?
                        JI_GESTURE_STATE_BEGAN : JI_GESTURE_STATE_CHANGED;
                    ev.x = cx;
                    ev.y = cy;
                    ev.scale = scale;
                    ev.touch_count = rec->touch_count;
                    ev.timestamp = p->timestamp;
                    ev.user_data = rec->user_data;
                    if (rec->callback) rec->callback(&ev, rec->user_data);
                    rec->state = ev.state;
                }
            }
            break;
        }

        case JI_GESTURE_ROTATE: {
            if (rec->touch_count >= 2) {
                float a = angle_between(rec->touches[0].x, rec->touches[0].y,
                                          rec->touches[1].x, rec->touches[1].y);
                float rotation = a - rec->initial_angle;
                if (fabsf(rotation) >= rec->config.rotate_min_angle) {
                    JiGestureEvent ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.type = rec->type;
                    ev.state = (rec->state == JI_GESTURE_STATE_POSSIBLE) ?
                        JI_GESTURE_STATE_BEGAN : JI_GESTURE_STATE_CHANGED;
                    ev.x = cx;
                    ev.y = cy;
                    ev.rotation = rotation;
                    ev.touch_count = rec->touch_count;
                    ev.timestamp = p->timestamp;
                    ev.user_data = rec->user_data;
                    if (rec->callback) rec->callback(&ev, rec->user_data);
                    rec->state = ev.state;
                }
            }
            break;
        }

        default:
            break;
    }

    rec->last_x = cx;
    rec->last_y = cy;
}

static void process_touch_up(JiGestureRecognizer* rec, uint32_t id, uint64_t ts) {
    /* Remove the touch */
    uint32_t i;
    for (i = 0; i < rec->touch_count; i++) {
        if (rec->touches[i].id == id) break;
    }
    if (i < rec->touch_count) {
        rec->touches[i] = rec->touches[rec->touch_count - 1];
        rec->touch_count--;
    }

    if (rec->touch_count == 0) {
        float cx = rec->last_x;
        float cy = rec->last_y;
        uint64_t duration = ts - rec->start_time;
        float total_dist = distance(rec->start_x, rec->start_y, cx, cy);

        switch (rec->type) {
            case JI_GESTURE_TAP:
                if (rec->state != JI_GESTURE_STATE_FAILED &&
                    duration <= rec->config.tap_max_duration &&
                    total_dist <= rec->config.tap_max_movement) {
                    gesture_recognizer_fire(rec, JI_GESTURE_STATE_ENDED, cx, cy, 0, 0);
                }
                break;

            case JI_GESTURE_DOUBLE_TAP:
                /* Simplified: fire on any quick tap */
                if (duration <= rec->config.tap_max_duration &&
                    total_dist <= rec->config.tap_max_movement) {
                    gesture_recognizer_fire(rec, JI_GESTURE_STATE_ENDED, cx, cy, 0, 0);
                }
                break;

            case JI_GESTURE_LONG_PRESS:
                if (rec->state == JI_GESTURE_STATE_BEGAN) {
                    gesture_recognizer_fire(rec, JI_GESTURE_STATE_ENDED, cx, cy, 0, 0);
                }
                break;

            case JI_GESTURE_DRAG:
            case JI_GESTURE_PAN:
                if (rec->state == JI_GESTURE_STATE_BEGAN || rec->state == JI_GESTURE_STATE_CHANGED) {
                    gesture_recognizer_fire(rec, JI_GESTURE_STATE_ENDED, cx, cy, 0, 0);
                }
                break;

            case JI_GESTURE_PINCH:
            case JI_GESTURE_ROTATE:
                if (rec->state == JI_GESTURE_STATE_BEGAN || rec->state == JI_GESTURE_STATE_CHANGED) {
                    gesture_recognizer_fire(rec, JI_GESTURE_STATE_ENDED, cx, cy, 0, 0);
                }
                break;

            default:
                break;
        }

        gesture_recognizer_reset(rec);
    }
}

/* =========================================================================
 * Gesture Manager
 * ========================================================================= */

struct JiGestureManager {
    JiGestureRecognizer** recognizers;
    uint32_t               rec_count;
    uint32_t               rec_capacity;
};

static void gesture_manager_ensure_capacity(JiGestureManager* mgr) {
    if (mgr->rec_count >= mgr->rec_capacity) {
        mgr->rec_capacity = mgr->rec_capacity ? mgr->rec_capacity * 2 : 8;
        mgr->recognizers = ji_realloc(mgr->recognizers, mgr->rec_capacity * sizeof(JiGestureRecognizer*));
    }
}

JI_API JiGestureManager* ji_gesture_manager_new(void) {
    return ji_calloc(1, sizeof(JiGestureManager));
}

JI_API void ji_gesture_manager_destroy(JiGestureManager* mgr) {
    if (!mgr) return;
    ji_free(mgr->recognizers);
    ji_free(mgr);
}

JI_API void ji_gesture_manager_add(JiGestureManager* mgr, JiGestureRecognizer* rec) {
    if (!mgr || !rec) return;
    gesture_manager_ensure_capacity(mgr);
    mgr->recognizers[mgr->rec_count++] = rec;
}

JI_API void ji_gesture_manager_remove(JiGestureManager* mgr, JiGestureRecognizer* rec) {
    if (!mgr || !rec) return;
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        if (mgr->recognizers[i] == rec) {
            mgr->recognizers[i] = mgr->recognizers[mgr->rec_count - 1];
            mgr->rec_count--;
            return;
        }
    }
}

JI_API void ji_gesture_manager_clear(JiGestureManager* mgr) {
    if (mgr) mgr->rec_count = 0;
}

JI_API uint32_t ji_gesture_manager_count(const JiGestureManager* mgr) {
    return mgr ? mgr->rec_count : 0;
}

JI_API void ji_gesture_manager_touch_down(JiGestureManager* mgr, const JiTouchPoint* point) {
    if (!mgr || !point) return;
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        process_touch_down(mgr->recognizers[i], point);
    }
}

JI_API void ji_gesture_manager_touch_move(JiGestureManager* mgr, const JiTouchPoint* point) {
    if (!mgr || !point) return;
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        process_touch_move(mgr->recognizers[i], point);
    }
}

JI_API void ji_gesture_manager_touch_up(JiGestureManager* mgr, uint32_t id, uint64_t timestamp) {
    if (!mgr) return;
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        process_touch_up(mgr->recognizers[i], id, timestamp);
    }
}

JI_API void ji_gesture_manager_touch_cancel(JiGestureManager* mgr, uint64_t timestamp) {
    if (!mgr) return;
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        gesture_recognizer_reset(mgr->recognizers[i]);
    }
}

JI_API void ji_gesture_manager_update(JiGestureManager* mgr, float dt) {
    if (!mgr) return;
    /* Apply inertia for drag/pan recognizers */
    for (uint32_t i = 0; i < mgr->rec_count; i++) {
        JiGestureRecognizer* rec = mgr->recognizers[i];
        if ((rec->type == JI_GESTURE_DRAG || rec->type == JI_GESTURE_PAN) &&
            rec->config.drag_inertia &&
            rec->state == JI_GESTURE_STATE_ENDED) {
            rec->velocity_x *= rec->config.drag_friction;
            rec->velocity_y *= rec->config.drag_friction;
            if (fabsf(rec->velocity_x) < 1.0f && fabsf(rec->velocity_y) < 1.0f) {
                rec->velocity_x = 0;
                rec->velocity_y = 0;
            }
        }
    }
}

/* =========================================================================
 * Custom Gesture Template
 * ========================================================================= */

struct JiGestureTemplate {
    char          name[64];
    JiTouchPoint* points;
    uint32_t      point_count;
    uint32_t      point_capacity;
};

JI_API JiGestureTemplate* ji_gesture_template_new(const char* name) {
    JiGestureTemplate* t = ji_calloc(1, sizeof(JiGestureTemplate));
    if (!t) return NULL;
    if (name) {
        strncpy(t->name, name, sizeof(t->name) - 1);
    }
    return t;
}

JI_API void ji_gesture_template_destroy(JiGestureTemplate* t) {
    if (!t) return;
    ji_free(t->points);
    ji_free(t);
}

JI_API void ji_gesture_template_add_point(JiGestureTemplate* t, float x, float y, uint64_t ts) {
    if (!t) return;
    if (t->point_count >= t->point_capacity) {
        t->point_capacity = t->point_capacity ? t->point_capacity * 2 : 32;
        t->points = ji_realloc(t->points, t->point_capacity * sizeof(JiTouchPoint));
    }
    t->points[t->point_count].id = 0;
    t->points[t->point_count].x = x;
    t->points[t->point_count].y = y;
    t->points[t->point_count].pressure = 1.0f;
    t->points[t->point_count].timestamp = ts;
    t->point_count++;
}

JI_API uint32_t ji_gesture_template_point_count(const JiGestureTemplate* t) {
    return t ? t->point_count : 0;
}

/* Resample a path of points into N evenly-spaced points.
 * Returns the resampled coordinates in out[][2]. */
static void resample_path(const float* xs, const float* ys, uint32_t pt_count,
                           float out[][2], uint32_t N) {
    /* Compute total path length */
    float total_len = 0;
    for (uint32_t i = 1; i < pt_count; i++) {
        total_len += distance(xs[i-1], ys[i-1], xs[i], ys[i]);
    }
    if (total_len < 1e-6f) {
        /* Degenerate path: all points at same location */
        for (uint32_t i = 0; i < N; i++) {
            out[i][0] = xs[0];
            out[i][1] = ys[0];
        }
        return;
    }

    float interval = total_len / (float)(N - 1);
    out[0][0] = xs[0];
    out[0][1] = ys[0];

    float accum = 0;          /* distance covered so far along the path */
    uint32_t seg = 0;          /* current segment index */

    for (uint32_t i = 1; i < N; i++) {
        float target = (float)i * interval;

        /* Advance segments until we reach or pass the target distance */
        while (seg < pt_count - 1 && accum + distance(xs[seg], ys[seg], xs[seg+1], ys[seg+1]) < target) {
            accum += distance(xs[seg], ys[seg], xs[seg+1], ys[seg+1]);
            seg++;
        }

        if (seg >= pt_count - 1) {
            /* Past the end — use last point */
            out[i][0] = xs[pt_count - 1];
            out[i][1] = ys[pt_count - 1];
        } else {
            float seg_len = distance(xs[seg], ys[seg], xs[seg+1], ys[seg+1]);
            if (seg_len < 1e-6f) {
                out[i][0] = xs[seg];
                out[i][1] = ys[seg];
            } else {
                float t = (target - accum) / seg_len;
                out[i][0] = xs[seg] + t * (xs[seg+1] - xs[seg]);
                out[i][1] = ys[seg] + t * (ys[seg+1] - ys[seg]);
            }
        }
    }
}

JI_API float ji_gesture_template_match(const JiGestureTemplate* tmpl,
                                   const JiTouchPoint* points, uint32_t count) {
    if (!tmpl || !points || tmpl->point_count == 0 || count == 0) return 0.0f;

    /* Simple $1 recognizer-style matching: resample both paths to N points,
     * compute average distance, convert to confidence. */
    const uint32_t N = 64;
    float tmpl_resampled[N][2];
    float input_resampled[N][2];

    /* Extract template coordinates into flat arrays */
    float tmpl_x[256], tmpl_y[256];
    uint32_t tcount = tmpl->point_count;
    if (tcount > 256) tcount = 256;
    for (uint32_t i = 0; i < tcount; i++) {
        tmpl_x[i] = tmpl->points[i].x;
        tmpl_y[i] = tmpl->points[i].y;
    }

    /* Extract input coordinates into flat arrays */
    float in_x[256], in_y[256];
    uint32_t icount = count;
    if (icount > 256) icount = 256;
    for (uint32_t i = 0; i < icount; i++) {
        in_x[i] = points[i].x;
        in_y[i] = points[i].y;
    }

    /* Check for degenerate input */
    float input_len = 0;
    for (uint32_t i = 1; i < icount; i++) {
        input_len += distance(in_x[i-1], in_y[i-1], in_x[i], in_y[i]);
    }
    if (input_len < 1.0f) return 0.0f;

    /* Resample both paths to N points */
    resample_path(tmpl_x, tmpl_y, tcount, tmpl_resampled, N);
    resample_path(in_x, in_y, icount, input_resampled, N);

    /* Compute average point distance between resampled paths */
    float total_dist = 0;
    for (uint32_t i = 0; i < N; i++) {
        total_dist += distance(tmpl_resampled[i][0], tmpl_resampled[i][1],
                                  input_resampled[i][0], input_resampled[i][1]);
    }
    float avg_dist = total_dist / N;

    /* Convert distance to confidence: closer = higher */
    float confidence = 1.0f / (1.0f + avg_dist / 100.0f);
    if (confidence > 1.0f) confidence = 1.0f;
    if (confidence < 0.0f) confidence = 0.0f;
    return confidence;
}

JI_API JiGestureRecognizer* ji_gesture_custom_new(const JiGestureTemplate* tmpl,
                                             float min_confidence) {
    JiGestureRecognizer* rec = ji_gesture_recognizer_new(JI_GESTURE_CUSTOM);
    if (!rec) return NULL;
    rec->template = tmpl;
    rec->min_confidence = min_confidence;
    return rec;
}
