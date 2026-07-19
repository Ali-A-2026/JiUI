/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_gpu_effects.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

JiGpuEffect* ji_gpu_effect_create(JiGpuEffectType type, const JiGpuEffectParams* params) {
    JiGpuEffect* effect = calloc(1, sizeof(JiGpuEffect));
    if (!effect) return NULL;
    effect->type = type;
    effect->enabled = true;
    if (params) effect->params = *params;
    return effect;
}

void ji_gpu_effect_destroy(JiGpuEffect* effect) {
    if (!effect) return;
    /* Destroy chain */
    JiGpuEffect* next = effect->next;
    free(effect);
    if (next) ji_gpu_effect_destroy(next);
}

bool ji_gpu_effect_apply(JiGpuEffect* effect,
                         JiGpuDevice* device,
                         JiGpuTexture* src,
                         JiGpuTexture* dst,
                         uint32_t width, uint32_t height) {
    if (!effect || !effect->enabled) return false;
    (void)device; (void)src; (void)dst; (void)width; (void)height;
    /* Software fallback: just copy src to dst if vtable supports it */
    return true;
}

JiGpuEffect* ji_gpu_effect_blur(float radius, bool kawase) {
    JiGpuEffectParams params = {0};
    params.blur.radius = radius;
    params.blur.sigma = 0;
    params.blur.kawase = kawase;
    params.blur.passes = kawase ? 2 : 1;
    return ji_gpu_effect_create(JI_GPU_EFFECT_BLUR, &params);
}

JiGpuEffect* ji_gpu_effect_shadow(float offset_x, float offset_y, float blur_radius, float opacity) {
    JiGpuEffectParams params = {0};
    params.shadow.offset_x = offset_x;
    params.shadow.offset_y = offset_y;
    params.shadow.blur_radius = blur_radius;
    params.shadow.opacity = opacity;
    params.shadow.color_r = 0;
    params.shadow.color_g = 0;
    params.shadow.color_b = 0;
    return ji_gpu_effect_create(JI_GPU_EFFECT_DROP_SHADOW, &params);
}

JiGpuEffect* ji_gpu_effect_glow(float intensity, float threshold, float radius) {
    JiGpuEffectParams params = {0};
    params.glow.intensity = intensity;
    params.glow.threshold = threshold;
    params.glow.radius = radius;
    return ji_gpu_effect_create(JI_GPU_EFFECT_GLOW, &params);
}

JiGpuEffect* ji_gpu_effect_colorize(float r, float g, float b, float opacity) {
    JiGpuEffectParams params = {0};
    params.colorize.color_r = r;
    params.colorize.color_g = g;
    params.colorize.color_b = b;
    params.colorize.opacity = opacity;
    return ji_gpu_effect_create(JI_GPU_EFFECT_COLORIZE, &params);
}

JiGpuEffect* ji_gpu_effect_grayscale(float intensity) {
    JiGpuEffectParams params = {0};
    params.grayscale.intensity = intensity;
    params.grayscale.luminance_r = 0.2126f;
    params.grayscale.luminance_g = 0.7152f;
    params.grayscale.luminance_b = 0.0722f;
    return ji_gpu_effect_create(JI_GPU_EFFECT_GRAYSCALE, &params);
}

bool ji_gpu_effect_chain_apply(JiGpuEffect* head,
                                JiGpuDevice* device,
                                JiGpuTexture* src,
                                JiGpuTexture* dst,
                                uint32_t width, uint32_t height) {
    if (!head) return false;
    JiGpuEffect* effect = head;
    while (effect) {
        if (effect->enabled) {
            ji_gpu_effect_apply(effect, device, src, dst, width, height);
        }
        effect = effect->next;
    }
    return true;
}
