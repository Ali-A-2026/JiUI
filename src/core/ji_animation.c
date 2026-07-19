/**
 * JiUI - Animation Framework implementation
 */

#include <jiui/ji_animation.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * Easing Curve
 * ========================================================================= */
JiEasingCurve ji_easing_curve_new(JiEasingType type) {
    JiEasingCurve c;
    memset(&c, 0, sizeof(c));
    c.type = type;
    c.cp1x = 0.25; c.cp1y = 0.1; c.cp2x = 0.25; c.cp2y = 1.0;
    c.spring_stiffness = 200.0;
    c.spring_damping = 20.0;
    c.spring_mass = 1.0;
    return c;
}

void ji_easing_curve_set_bezier(JiEasingCurve* curve, double cp1x, double cp1y, double cp2x, double cp2y) {
    if (!curve) return;
    curve->type = JI_EASE_CUBIC_BEZIER;
    curve->cp1x = cp1x; curve->cp1y = cp1y;
    curve->cp2x = cp2x; curve->cp2y = cp2y;
}

void ji_easing_curve_set_spring(JiEasingCurve* curve, double stiffness, double damping, double mass) {
    if (!curve) return;
    curve->type = JI_EASE_SPRING;
    curve->spring_stiffness = stiffness;
    curve->spring_damping = damping;
    curve->spring_mass = mass;
}

static double ease_out_bounce(double t) {
    if (t < 1.0/2.75) return 7.5625*t*t;
    else if (t < 2.0/2.75) { t -= 1.5/2.75; return 7.5625*t*t + 0.75; }
    else if (t < 2.5/2.75) { t -= 2.25/2.75; return 7.5625*t*t + 0.9375; }
    else { t -= 2.625/2.75; return 7.5625*t*t + 0.984375; }
}

static double ease_in_bounce(double t) { return 1.0 - ease_out_bounce(1.0 - t); }

static double ease_elastic(double t, bool out) {
    if (t == 0.0 || t == 1.0) return t;
    double p = 0.3;
    double s = p / 4.0;
    if (out) return pow(2.0, -10.0*t) * sin((t - s) * 2.0 * M_PI / p) + 1.0;
    return -pow(2.0, 10.0*(t-1.0)) * sin((t - 1.0 - s) * 2.0 * M_PI / p);
}

/* Simple cubic bezier evaluation using De Casteljau */
static double cubic_bezier(double t, double p1, double p2) {
    /* Approximate with 8 iterations of Newton-Raphson for x, then evaluate y */
    double x = t;
    for (int i = 0; i < 8; i++) {
        double x0 = 3.0*(1.0-x)*(1.0-x)*x*p1 + 3.0*(1.0-x)*x*x*p2 + x*x*x;
        double dx = 3.0*(1.0-x)*(1.0-x)*p1 + 6.0*(1.0-x)*x*(p2-p1) + 3.0*x*x*(1.0-p2);
        if (fabs(dx) < 1e-10) break;
        x -= (x0 - t) / dx;
    }
    return 3.0*(1.0-x)*(1.0-x)*x*1.0 + 3.0*(1.0-x)*x*x*1.0 + x*x*x;
}

double ji_easing_curve_value(const JiEasingCurve* curve, double progress) {
    if (!curve || progress <= 0.0) return 0.0;
    if (progress >= 1.0) return 1.0;
    double t = progress;
    switch (curve->type) {
        case JI_EASE_LINEAR: return t;
        case JI_EASE_IN_QUAD: return t*t;
        case JI_EASE_OUT_QUAD: return t*(2.0-t);
        case JI_EASE_IN_OUT_QUAD: return t < 0.5 ? 2.0*t*t : -1.0+(4.0-2.0*t)*t;
        case JI_EASE_IN_CUBIC: return t*t*t;
        case JI_EASE_OUT_CUBIC: return (--t)*t*t+1.0;
        case JI_EASE_IN_OUT_CUBIC: return t<0.5 ? 4.0*t*t*t : (t-1.0)*(2.0*t-2.0)*(2.0*t-2.0)+1.0;
        case JI_EASE_IN_QUART: return t*t*t*t;
        case JI_EASE_OUT_QUART: return 1.0-(--t)*t*t*t;
        case JI_EASE_IN_OUT_QUART: return t<0.5 ? 8.0*t*t*t*t : 1.0-8.0*(--t)*t*t*t;
        case JI_EASE_IN_QUINT: return t*t*t*t*t;
        case JI_EASE_OUT_QUINT: return 1.0+(--t)*t*t*t*t;
        case JI_EASE_IN_OUT_QUINT: return t<0.5 ? 16.0*t*t*t*t*t : 1.0+16.0*(--t)*t*t*t*t;
        case JI_EASE_IN_SINE: return 1.0-cos(t*M_PI/2.0);
        case JI_EASE_OUT_SINE: return sin(t*M_PI/2.0);
        case JI_EASE_IN_OUT_SINE: return 0.5*(1.0-cos(t*M_PI));
        case JI_EASE_IN_EXPO: return t==0.0 ? 0.0 : pow(2.0, 10.0*(t-1.0));
        case JI_EASE_OUT_EXPO: return t==1.0 ? 1.0 : 1.0-pow(2.0, -10.0*t);
        case JI_EASE_IN_OUT_EXPO: return t==0.0?0.0:t==1.0?1.0:t<0.5?pow(2.0,20.0*t-10.0)/2.0:(2.0-pow(2.0,-20.0*t+10.0))/2.0;
        case JI_EASE_IN_CIRC: return 1.0-sqrt(1.0-t*t);
        case JI_EASE_OUT_CIRC: return sqrt(1.0-(--t)*t);
        case JI_EASE_IN_OUT_CIRC: return t<0.5?(1.0-sqrt(1.0-4.0*t*t))/2.0:(sqrt(1.0-(--t)*2.0*(t-2.0))+1.0)/2.0;
        case JI_EASE_IN_BACK: { double s=1.70158; return t*t*((s+1.0)*t-s); }
        case JI_EASE_OUT_BACK: { double s=1.70158; return (--t)*t*((s+1.0)*t+s)+1.0; }
        case JI_EASE_IN_OUT_BACK: { double s=1.70158*1.525; return t<0.5?(4.0*t*t*((s+1.0)*2.0*t-s))/2.0:((4.0*(--t)*t*((s+1.0)*2.0*t+s))+2.0)/2.0; }
        case JI_EASE_IN_ELASTIC: return ease_elastic(t, false);
        case JI_EASE_OUT_ELASTIC: return ease_elastic(t, true);
        case JI_EASE_IN_OUT_ELASTIC: return t<0.5 ? 0.5*ease_elastic(t*2.0, false) : 0.5+0.5*ease_elastic(t*2.0-1.0, true);
        case JI_EASE_IN_BOUNCE: return ease_in_bounce(t);
        case JI_EASE_OUT_BOUNCE: return ease_out_bounce(t);
        case JI_EASE_IN_OUT_BOUNCE: return t<0.5 ? 0.5*ease_in_bounce(t*2.0) : 0.5+0.5*ease_out_bounce(t*2.0-1.0);
        case JI_EASE_CUBIC_BEZIER: return cubic_bezier(t, curve->cp1y, curve->cp2y);
        case JI_EASE_SPRING: {
            double w = sqrt(curve->spring_stiffness / curve->spring_mass);
            double d = curve->spring_damping / (2.0 * sqrt(curve->spring_stiffness * curve->spring_mass));
            if (d < 1.0) {
                double wd = w * sqrt(1.0 - d*d);
                return 1.0 - exp(-d*w*t*3.0) * (cos(wd*t*3.0) + (d*w)/(wd)*sin(wd*t*3.0));
            }
            return 1.0 - exp(-w*t*3.0) * (1.0 + w*t*3.0);
        }
    }
    return t;
}

/* =========================================================================
 * Animation base
 * ========================================================================= */
void ji_animation_init(JiAnimation* anim) {
    if (!anim) return;
    memset(anim, 0, sizeof(*anim));
    anim->duration_ms = 250;
    anim->loop_count = 1;
    anim->direction = JI_ANIMATION_FORWARD;
    anim->easing = ji_easing_curve_new(JI_EASE_LINEAR);
    anim->current_progress = 0.0;
    anim->state = JI_ANIMATION_STOPPED;
}

void ji_animation_set_duration(JiAnimation* anim, int ms) { if (anim) anim->duration_ms = ms; }
void ji_animation_set_loop_count(JiAnimation* anim, int count) { if (anim) anim->loop_count = count; }
void ji_animation_set_direction(JiAnimation* anim, JiAnimationDirection dir) { if (anim) anim->direction = dir; }
void ji_animation_set_easing(JiAnimation* anim, JiEasingCurve easing) { if (anim) anim->easing = easing; }

void ji_animation_start(JiAnimation* anim) {
    if (!anim) return;
    anim->state = JI_ANIMATION_RUNNING;
    anim->current_progress = 0.0;
    anim->current_loop = 0;
    anim->start_time_ms = 0; /* caller should set */
}

void ji_animation_pause(JiAnimation* anim) {
    if (!anim || anim->state != JI_ANIMATION_RUNNING) return;
    anim->state = JI_ANIMATION_PAUSED;
    anim->pause_time_ms = 0;
}

void ji_animation_resume(JiAnimation* anim) {
    if (!anim || anim->state != JI_ANIMATION_PAUSED) return;
    anim->state = JI_ANIMATION_RUNNING;
}

void ji_animation_stop(JiAnimation* anim) {
    if (!anim) return;
    anim->state = JI_ANIMATION_STOPPED;
    anim->current_progress = 0.0;
}

double ji_animation_current_value(const JiAnimation* anim) {
    return anim ? ji_easing_curve_value(&anim->easing, anim->current_progress) : 0.0;
}

bool ji_animation_update(JiAnimation* anim, uint64_t current_time_ms) {
    if (!anim || anim->state != JI_ANIMATION_RUNNING) return false;
    if (anim->duration_ms <= 0) { anim->current_progress = 1.0; anim->state = JI_ANIMATION_STOPPED; return true; }
    uint64_t elapsed = current_time_ms - anim->start_time_ms;
    double raw = (double)elapsed / (double)anim->duration_ms;
    if (raw >= 1.0) {
        anim->current_loop++;
        if (anim->loop_count > 0 && anim->current_loop >= anim->loop_count) {
            anim->current_progress = 1.0;
            anim->state = JI_ANIMATION_STOPPED;
            if (anim->on_finished) anim->on_finished(anim, anim->user_data);
            return true;
        }
        raw = fmod(raw, 1.0);
    }
    /* Apply direction */
    bool reverse = false;
    switch (anim->direction) {
        case JI_ANIMATION_REVERSE: reverse = true; break;
        case JI_ANIMATION_ALTERNATE: reverse = (anim->current_loop % 2 == 1); break;
        case JI_ANIMATION_ALTERNATE_REVERSE: reverse = (anim->current_loop % 2 == 0); break;
        default: break;
    }
    anim->current_progress = reverse ? 1.0 - raw : raw;
    return true;
}

/* =========================================================================
 * Property Animation
 * ========================================================================= */
JiPropertyAnimation* ji_property_animation_new(void) {
    JiPropertyAnimation* pa = (JiPropertyAnimation*)ji_calloc(1, sizeof(JiPropertyAnimation));
    if (!pa) { JI_ERROR_LOG("ji_property_animation_new: out of memory"); return NULL; }
    ji_animation_init(&pa->base);
    return pa;
}

void ji_property_animation_destroy(JiPropertyAnimation* anim) { if (anim) ji_free(anim); }
void ji_property_animation_set_range(JiPropertyAnimation* anim, double start, double end) {
    if (!anim) return;
    anim->start_value = start;
    anim->end_value = end;
}

/* =========================================================================
 * Animation Group
 * ========================================================================= */
JiAnimationGroup* ji_animation_group_new(JiAnimationGroupMode mode) {
    JiAnimationGroup* g = (JiAnimationGroup*)ji_calloc(1, sizeof(JiAnimationGroup));
    if (!g) { JI_ERROR_LOG("ji_animation_group_new: out of memory"); return NULL; }
    ji_animation_init(&g->base);
    g->mode = mode;
    g->child_capacity = 4;
    g->children = (JiAnimation**)ji_alloc(sizeof(JiAnimation*) * g->child_capacity);
    return g;
}

void ji_animation_group_destroy(JiAnimationGroup* group) { if (group) { ji_free(group->children); ji_free(group); } }

void ji_animation_group_add(JiAnimationGroup* group, JiAnimation* anim) {
    if (!group || !anim) return;
    if (group->child_count >= group->child_capacity) {
        group->child_capacity *= 2;
        JiAnimation** new_arr = (JiAnimation**)ji_alloc(sizeof(JiAnimation*) * group->child_capacity);
        if (!new_arr) return;
        memcpy(new_arr, group->children, sizeof(JiAnimation*) * group->child_count);
        ji_free(group->children);
        group->children = new_arr;
    }
    group->children[group->child_count++] = anim;
}

void ji_animation_group_remove(JiAnimationGroup* group, JiAnimation* anim) {
    if (!group || !anim) return;
    for (int i = 0; i < group->child_count; i++) {
        if (group->children[i] == anim) {
            for (int j = i; j < group->child_count - 1; j++) group->children[j] = group->children[j+1];
            group->child_count--;
            return;
        }
    }
}

/* =========================================================================
 * Animation Driver
 * ========================================================================= */
JiAnimationDriver* ji_animation_driver_new(void) {
    JiAnimationDriver* d = (JiAnimationDriver*)ji_calloc(1, sizeof(JiAnimationDriver));
    if (!d) { JI_ERROR_LOG("ji_animation_driver_new: out of memory"); return NULL; }
    d->animation_capacity = 16;
    d->animations = (JiAnimation**)ji_alloc(sizeof(JiAnimation*) * d->animation_capacity);
    return d;
}

void ji_animation_driver_destroy(JiAnimationDriver* driver) { if (driver) { ji_free(driver->animations); ji_free(driver); } }

void ji_animation_driver_register(JiAnimationDriver* driver, JiAnimation* anim) {
    if (!driver || !anim) return;
    if (driver->animation_count >= driver->animation_capacity) {
        driver->animation_capacity *= 2;
        JiAnimation** new_arr = (JiAnimation**)ji_alloc(sizeof(JiAnimation*) * driver->animation_capacity);
        if (!new_arr) return;
        memcpy(new_arr, driver->animations, sizeof(JiAnimation*) * driver->animation_count);
        ji_free(driver->animations);
        driver->animations = new_arr;
    }
    driver->animations[driver->animation_count++] = anim;
}

void ji_animation_driver_unregister(JiAnimationDriver* driver, JiAnimation* anim) {
    if (!driver || !anim) return;
    for (int i = 0; i < driver->animation_count; i++) {
        if (driver->animations[i] == anim) {
            for (int j = i; j < driver->animation_count - 1; j++) driver->animations[j] = driver->animations[j+1];
            driver->animation_count--;
            return;
        }
    }
}

void ji_animation_driver_tick(JiAnimationDriver* driver, uint64_t current_time_ms) {
    if (!driver) return;
    for (int i = 0; i < driver->animation_count; i++) {
        JiAnimation* anim = driver->animations[i];
        if (anim->state == JI_ANIMATION_RUNNING) {
            if (anim->start_time_ms == 0) anim->start_time_ms = current_time_ms;
            ji_animation_update(anim, current_time_ms);
        }
    }
}
