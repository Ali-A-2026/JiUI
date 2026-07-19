/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_gpu_effects.h
 * @brief GPU-accelerated visual effects — blur, shadow, glow, colorize,
 *        grayscale, and custom shader effects.
 */

#ifndef JIUI_GPU_EFFECTS_H
#define JIUI_GPU_EFFECTS_H

#include "ji_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Effect IDs
 * ========================================================================= */

typedef enum JiGpuEffectType {
    JI_GPU_EFFECT_BLUR,         /* Separable Gaussian / Kawase blur */
    JI_GPU_EFFECT_DROP_SHADOW,  /* Single-pass drop shadow */
    JI_GPU_EFFECT_GLOW,         /* Bloom / glow pass */
    JI_GPU_EFFECT_COLORIZE,      /* Tint with color + alpha */
    JI_GPU_EFFECT_GRAYSCALE,     /* Luminance conversion */
    JI_GPU_EFFECT_CUSTOM        /* User-provided shader */
} JiGpuEffectType;

/* =========================================================================
 * Effect Parameters
 * ========================================================================= */

typedef struct JiBlurParams {
    float   radius;         /* Blur radius in pixels (0–64) */
    float   sigma;          /* Gaussian sigma (0 = auto from radius) */
    bool    kawase;         /* Use Kawase blur instead of Gaussian */
    uint32_t passes;        /* Number of blur passes (1–4) */
} JiBlurParams;

typedef struct JiShadowParams {
    float   offset_x;       /* Shadow offset X */
    float   offset_y;       /* Shadow offset Y */
    float   blur_radius;    /* Shadow blur radius */
    float   opacity;         /* Shadow opacity (0–1) */
    float   color_r, color_g, color_b;  /* Shadow color */
} JiShadowParams;

typedef struct JiGlowParams {
    float   intensity;      /* Glow intensity (0–2) */
    float   threshold;      /* Brightness threshold for bloom */
    float   radius;         /* Bloom spread radius */
} JiGlowParams;

typedef struct JiColorizeParams {
    float   color_r, color_g, color_b;  /* Tint color */
    float   opacity;         /* Tint opacity (0–1) */
} JiColorizeParams;

typedef struct JiGrayscaleParams {
    float   intensity;      /* 0 = original, 1 = full grayscale */
    float   luminance_r;    /* Red weight (default 0.2126) */
    float   luminance_g;    /* Green weight (default 0.7152) */
    float   luminance_b;    /* Blue weight (default 0.0722) */
} JiGrayscaleParams;

typedef struct JiCustomEffectParams {
    void*   shader;         /* User-provided shader bytecode */
    uint32_t shader_size;    /* Shader bytecode size */
    void*   uniform_data;   /* Custom uniform data */
    uint32_t uniform_size;   /* Custom uniform data size */
} JiCustomEffectParams;

typedef union JiGpuEffectParams {
    JiBlurParams       blur;
    JiShadowParams     shadow;
    JiGlowParams       glow;
    JiColorizeParams   colorize;
    JiGrayscaleParams  grayscale;
    JiCustomEffectParams custom;
} JiGpuEffectParams;

/* =========================================================================
 * GPU Effect
 * ========================================================================= */

typedef struct JiGpuEffect JiGpuEffect;

struct JiGpuEffect {
    JiGpuEffectType    type;
    JiGpuEffectParams  params;
    bool               enabled;
    JiGpuEffect*       next;       /* Effect chain linked list */
};

/* Create/destroy */
JiGpuEffect* ji_gpu_effect_create(JiGpuEffectType type, const JiGpuEffectParams* params);
void          ji_gpu_effect_destroy(JiGpuEffect* effect);

/* Apply effect to a render target, output to dst */
bool          ji_gpu_effect_apply(JiGpuEffect* effect,
                                   JiGpuDevice* device,
                                   JiGpuTexture* src,
                                   JiGpuTexture* dst,
                                   uint32_t width, uint32_t height);

/* Convenience constructors */
JiGpuEffect* ji_gpu_effect_blur(float radius, bool kawase);
JiGpuEffect* ji_gpu_effect_shadow(float offset_x, float offset_y, float blur_radius, float opacity);
JiGpuEffect* ji_gpu_effect_glow(float intensity, float threshold, float radius);
JiGpuEffect* ji_gpu_effect_colorize(float r, float g, float b, float opacity);
JiGpuEffect* ji_gpu_effect_grayscale(float intensity);

/* Effect chain execution */
bool          ji_gpu_effect_chain_apply(JiGpuEffect* head,
                                         JiGpuDevice* device,
                                         JiGpuTexture* src,
                                         JiGpuTexture* dst,
                                         uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_GPU_EFFECTS_H */
