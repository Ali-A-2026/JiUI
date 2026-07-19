/**
 * JiUI - Graphics Effects Framework implementation
 */

#include <jiui/ji_effects.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- Base Effect ---- */
void ji_effect_init(JiEffect* effect, JiEffectType type) {
    if (!effect) return;
    memset(effect, 0, sizeof(JiEffect));
    effect->type = type;
    effect->is_enabled = true;
    effect->strength = 1.0;
}

void ji_effect_set_enabled(JiEffect* effect, bool enabled) { if (effect) effect->is_enabled = enabled; }
void ji_effect_set_strength(JiEffect* effect, double strength) {
    if (!effect) return;
    if (strength < 0.0) strength = 0.0;
    if (strength > 1.0) strength = 1.0;
    effect->strength = strength;
}

/* ---- Blur Effect ---- */
JiBlurEffect* ji_blur_effect_new(void) {
    JiBlurEffect* e = (JiBlurEffect*)ji_calloc(1, sizeof(JiBlurEffect));
    if (!e) { JI_ERROR_LOG("ji_blur_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_BLUR);
    e->radius = 5;
    e->is_high_quality = false;
    e->base.padding_left = e->base.padding_right = e->base.padding_top = e->base.padding_bottom = 5;
    return e;
}

void ji_blur_effect_destroy(JiBlurEffect* effect) { ji_free(effect); }
void ji_blur_effect_set_radius(JiBlurEffect* effect, int radius) {
    if (!effect) return;
    effect->radius = radius < 1 ? 1 : (radius > 100 ? 100 : radius);
    effect->base.padding_left = effect->base.padding_right = effect->base.padding_top = effect->base.padding_bottom = effect->radius;
}
void ji_blur_effect_set_quality(JiBlurEffect* effect, bool high_quality) { if (effect) effect->is_high_quality = high_quality; }

/* ---- Drop Shadow Effect ---- */
JiDropShadowEffect* ji_drop_shadow_effect_new(void) {
    JiDropShadowEffect* e = (JiDropShadowEffect*)ji_calloc(1, sizeof(JiDropShadowEffect));
    if (!e) { JI_ERROR_LOG("ji_drop_shadow_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_DROP_SHADOW);
    e->offset_x = 1; e->offset_y = 1;
    e->blur_radius = 5;
    e->color = 0x7F000000;
    e->is_inner_shadow = false;
    e->base.padding_left = e->base.padding_right = e->base.padding_top = e->base.padding_bottom = 6;
    return e;
}

void ji_drop_shadow_effect_destroy(JiDropShadowEffect* effect) { ji_free(effect); }
void ji_drop_shadow_set_offset(JiDropShadowEffect* effect, int x, int y) { if (effect) { effect->offset_x = x; effect->offset_y = y; } }
void ji_drop_shadow_set_blur_radius(JiDropShadowEffect* effect, int radius) { if (effect) effect->blur_radius = radius; }
void ji_drop_shadow_set_color(JiDropShadowEffect* effect, uint32_t argb) { if (effect) effect->color = argb; }
void ji_drop_shadow_set_inner(JiDropShadowEffect* effect, bool inner) { if (effect) effect->is_inner_shadow = inner; }

/* ---- Opacity Effect ---- */
JiOpacityEffect* ji_opacity_effect_new(void) {
    JiOpacityEffect* e = (JiOpacityEffect*)ji_calloc(1, sizeof(JiOpacityEffect));
    if (!e) { JI_ERROR_LOG("ji_opacity_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_OPACITY);
    e->opacity = 1.0;
    return e;
}

void ji_opacity_effect_destroy(JiOpacityEffect* effect) { ji_free(effect); }
void ji_opacity_effect_set_opacity(JiOpacityEffect* effect, double opacity) {
    if (!effect) return;
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;
    effect->opacity = opacity;
}

/* ---- Colorize Effect ---- */
JiColorizeEffect* ji_colorize_effect_new(void) {
    JiColorizeEffect* e = (JiColorizeEffect*)ji_calloc(1, sizeof(JiColorizeEffect));
    if (!e) { JI_ERROR_LOG("ji_colorize_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_COLORIZE);
    e->color = 0xFFFFFFFF;
    e->intensity = 0.5;
    return e;
}

void ji_colorize_effect_destroy(JiColorizeEffect* effect) { ji_free(effect); }
void ji_colorize_effect_set_color(JiColorizeEffect* effect, uint32_t argb) { if (effect) effect->color = argb; }
void ji_colorize_effect_set_intensity(JiColorizeEffect* effect, double intensity) {
    if (!effect) return;
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 1.0) intensity = 1.0;
    effect->intensity = intensity;
}

/* ---- Glow Effect ---- */
JiGlowEffect* ji_glow_effect_new(void) {
    JiGlowEffect* e = (JiGlowEffect*)ji_calloc(1, sizeof(JiGlowEffect));
    if (!e) { JI_ERROR_LOG("ji_glow_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_GLOW);
    e->color = 0x7FFFFFFF;
    e->spread = 5;
    e->intensity = 0.5;
    e->base.padding_left = e->base.padding_right = e->base.padding_top = e->base.padding_bottom = 5;
    return e;
}

void ji_glow_effect_destroy(JiGlowEffect* effect) { ji_free(effect); }
void ji_glow_effect_set_color(JiGlowEffect* effect, uint32_t argb) { if (effect) effect->color = argb; }
void ji_glow_effect_set_spread(JiGlowEffect* effect, int spread) {
    if (!effect) return;
    effect->spread = spread;
    effect->base.padding_left = effect->base.padding_right = effect->base.padding_top = effect->base.padding_bottom = spread;
}
void ji_glow_effect_set_intensity(JiGlowEffect* effect, double intensity) {
    if (!effect) return;
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 1.0) intensity = 1.0;
    effect->intensity = intensity;
}

/* ---- Grayscale Effect ---- */
JiGrayscaleEffect* ji_grayscale_effect_new(void) {
    JiGrayscaleEffect* e = (JiGrayscaleEffect*)ji_calloc(1, sizeof(JiGrayscaleEffect));
    if (!e) { JI_ERROR_LOG("ji_grayscale_effect_new: out of memory"); return NULL; }
    ji_effect_init(&e->base, JI_EFFECT_GRAYSCALE);
    e->amount = 1.0;
    return e;
}

void ji_grayscale_effect_destroy(JiGrayscaleEffect* effect) { ji_free(effect); }
void ji_grayscale_effect_set_amount(JiGrayscaleEffect* effect, double amount) {
    if (!effect) return;
    if (amount < 0.0) amount = 0.0;
    if (amount > 1.0) amount = 1.0;
    effect->amount = amount;
}

/* ---- Effect Chain ---- */
JiEffectChain* ji_effect_chain_new(void) {
    JiEffectChain* c = (JiEffectChain*)ji_calloc(1, sizeof(JiEffectChain));
    if (!c) { JI_ERROR_LOG("ji_effect_chain_new: out of memory"); return NULL; }
    c->effect_capacity = 4;
    c->effects = (JiEffect**)ji_alloc(sizeof(JiEffect*) * c->effect_capacity);
    return c;
}

void ji_effect_chain_destroy(JiEffectChain* chain) { if (chain) { ji_free(chain->effects); ji_free(chain); } }

void ji_effect_chain_add(JiEffectChain* chain, JiEffect* effect) {
    if (!chain || !effect) return;
    if (chain->effect_count >= chain->effect_capacity) {
        chain->effect_capacity *= 2;
        JiEffect** new_arr = (JiEffect**)ji_alloc(sizeof(JiEffect*) * chain->effect_capacity);
        if (!new_arr) return;
        memcpy(new_arr, chain->effects, sizeof(JiEffect*) * chain->effect_count);
        ji_free(chain->effects);
        chain->effects = new_arr;
    }
    chain->effects[chain->effect_count++] = effect;
}

void ji_effect_chain_remove(JiEffectChain* chain, JiEffect* effect) {
    if (!chain || !effect) return;
    for (int i = 0; i < chain->effect_count; i++) {
        if (chain->effects[i] == effect) {
            for (int j = i; j < chain->effect_count - 1; j++) chain->effects[j] = chain->effects[j+1];
            chain->effect_count--;
            return;
        }
    }
}

void ji_effect_chain_clear(JiEffectChain* chain) { if (chain) chain->effect_count = 0; }
int ji_effect_chain_count(const JiEffectChain* chain) { return chain ? chain->effect_count : 0; }
