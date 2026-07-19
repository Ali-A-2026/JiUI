/**
 * JiUI - Graphics Effects Framework header
 * Provides real-time visual effects: blur, shadow, opacity, colorize, glow.
 * Surpasses Qt6 with GPU-accelerated effect chains and live preview.
 */

#ifndef JIUI_EFFECTS_H
#define JIUI_EFFECTS_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Effect Types
 * ========================================================================= */
typedef enum JiEffectType {
    JI_EFFECT_BLUR = 0,
    JI_EFFECT_DROP_SHADOW,
    JI_EFFECT_OPACITY,
    JI_EFFECT_COLORIZE,
    JI_EFFECT_GLOW,
    JI_EFFECT_GRAYSCALE,
    JI_EFFECT_CUSTOM
} JiEffectType;

/* =========================================================================
 * Base Effect
 * ========================================================================= */
typedef struct JiEffect {
    JiEffectType   type;
    bool           is_enabled;
    double         strength;       /* 0.0 - 1.0, default 1.0 */
    int            padding_left;   /* extra space needed for the effect */
    int            padding_top;
    int            padding_right;
    int            padding_bottom;
} JiEffect;

JI_API void ji_effect_init(JiEffect* effect, JiEffectType type);
JI_API void ji_effect_set_enabled(JiEffect* effect, bool enabled);
JI_API void ji_effect_set_strength(JiEffect* effect, double strength);

/* =========================================================================
 * Blur Effect — Gaussian blur with configurable radius
 * ========================================================================= */
typedef struct JiBlurEffect {
    JiEffect   base;
    int        radius;          /* default 5, range 1-100 */
    bool       is_high_quality; /* uses two-pass separable blur */
} JiBlurEffect;

JI_API JiBlurEffect* ji_blur_effect_new(void);
JI_API void ji_blur_effect_destroy(JiBlurEffect* effect);
JI_API void ji_blur_effect_set_radius(JiBlurEffect* effect, int radius);
JI_API void ji_blur_effect_set_quality(JiBlurEffect* effect, bool high_quality);

/* =========================================================================
 * Drop Shadow Effect — shadow with offset, radius, color
 * ========================================================================= */
typedef struct JiDropShadowEffect {
    JiEffect   base;
    int        offset_x;        /* default 1 */
    int        offset_y;        /* default 1 */
    int        blur_radius;     /* default 5 */
    uint32_t   color;           /* ARGB, default 0x7F000000 */
    bool       is_inner_shadow; /* beyond Qt6: inner shadow mode */
} JiDropShadowEffect;

JI_API JiDropShadowEffect* ji_drop_shadow_effect_new(void);
JI_API void ji_drop_shadow_effect_destroy(JiDropShadowEffect* effect);
JI_API void ji_drop_shadow_set_offset(JiDropShadowEffect* effect, int x, int y);
JI_API void ji_drop_shadow_set_blur_radius(JiDropShadowEffect* effect, int radius);
JI_API void ji_drop_shadow_set_color(JiDropShadowEffect* effect, uint32_t argb);
JI_API void ji_drop_shadow_set_inner(JiDropShadowEffect* effect, bool inner);

/* =========================================================================
 * Opacity Effect — adjusts transparency
 * ========================================================================= */
typedef struct JiOpacityEffect {
    JiEffect   base;
    double     opacity;          /* 0.0 (transparent) - 1.0 (opaque) */
} JiOpacityEffect;

JI_API JiOpacityEffect* ji_opacity_effect_new(void);
JI_API void ji_opacity_effect_destroy(JiOpacityEffect* effect);
JI_API void ji_opacity_effect_set_opacity(JiOpacityEffect* effect, double opacity);

/* =========================================================================
 * Colorize Effect — tints with a color and strength
 * ========================================================================= */
typedef struct JiColorizeEffect {
    JiEffect   base;
    uint32_t   color;           /* ARGB tint color */
    double     intensity;       /* 0.0 - 1.0, default 0.5 */
} JiColorizeEffect;

JI_API JiColorizeEffect* ji_colorize_effect_new(void);
JI_API void ji_colorize_effect_destroy(JiColorizeEffect* effect);
JI_API void ji_colorize_effect_set_color(JiColorizeEffect* effect, uint32_t argb);
JI_API void ji_colorize_effect_set_intensity(JiColorizeEffect* effect, double intensity);

/* =========================================================================
 * Glow Effect — outer glow with color and spread (beyond Qt6)
 * ========================================================================= */
typedef struct JiGlowEffect {
    JiEffect   base;
    uint32_t   color;           /* ARGB glow color, default 0x7FFFFFFF */
    int        spread;          /* glow spread in pixels, default 5 */
    double     intensity;       /* 0.0 - 1.0, default 0.5 */
} JiGlowEffect;

JI_API JiGlowEffect* ji_glow_effect_new(void);
JI_API void ji_glow_effect_destroy(JiGlowEffect* effect);
JI_API void ji_glow_effect_set_color(JiGlowEffect* effect, uint32_t argb);
JI_API void ji_glow_effect_set_spread(JiGlowEffect* effect, int spread);
JI_API void ji_glow_effect_set_intensity(JiGlowEffect* effect, double intensity);

/* =========================================================================
 * Grayscale Effect — desaturates to grayscale
 * ========================================================================= */
typedef struct JiGrayscaleEffect {
    JiEffect   base;
    double     amount;          /* 0.0 (full color) - 1.0 (full gray) */
} JiGrayscaleEffect;

JI_API JiGrayscaleEffect* ji_grayscale_effect_new(void);
JI_API void ji_grayscale_effect_destroy(JiGrayscaleEffect* effect);
JI_API void ji_grayscale_effect_set_amount(JiGrayscaleEffect* effect, double amount);

/* =========================================================================
 * Effect Chain — applies multiple effects in sequence (beyond Qt6)
 * ========================================================================= */
typedef struct JiEffectChain {
    JiEffect**  effects;
    int         effect_count;
    int         effect_capacity;
} JiEffectChain;

JI_API JiEffectChain* ji_effect_chain_new(void);
JI_API void ji_effect_chain_destroy(JiEffectChain* chain);
JI_API void ji_effect_chain_add(JiEffectChain* chain, JiEffect* effect);
JI_API void ji_effect_chain_remove(JiEffectChain* chain, JiEffect* effect);
JI_API void ji_effect_chain_clear(JiEffectChain* chain);
JI_API int ji_effect_chain_count(const JiEffectChain* chain);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_EFFECTS_H */
