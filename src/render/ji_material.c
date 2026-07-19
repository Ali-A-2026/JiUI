/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_material.c
 * @brief JiUI Material Engine — implementation of material creation, properties,
 *        transitions, and the material engine lifecycle.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================================
 * Internal: transition state (stored in normal_map pointer slot when transitioning)
 * ========================================================================= */

typedef struct JiMaterialTransition {
    JiMaterialType   from_type;
    JiMaterialType   to_type;
    float            duration;       /* total duration in seconds */
    float            elapsed;        /* elapsed time in seconds */
    bool             active;
    /* Saved properties from the original material for interpolation */
    JiVec4           from_base_color;
    float            from_metallic;
    float            from_roughness;
    float            from_emissive;
    float            from_clearcoat;
    float            from_clearcoat_roughness;
    float            from_anisotropy;
    float            from_ior;
    float            from_blur_amount;
    float            from_bloom_threshold;
    float            from_holographic_shift;
} JiMaterialTransition;

/* =========================================================================
 * Helpers
 * ========================================================================= */

static float ji_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float ji_lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

static JiVec4 ji_vec4_lerp(JiVec4 a, JiVec4 b, float t) {
    JiVec4 r;
    r.x = ji_lerpf(a.x, b.x, t);
    r.y = ji_lerpf(a.y, b.y, t);
    r.z = ji_lerpf(a.z, b.z, t);
    r.w = ji_lerpf(a.w, b.w, t);
    return r;
}

static float ji_ease_in_out(float t) {
    return t * t * (3.0f - 2.0f * t);
}

/* Default property values per material type */
static void ji_material_apply_defaults(JiUIMaterial* mat) {
    if (!mat) return;

    switch (mat->type) {
    case JI_MATERIAL_FLAT:
        mat->metallic           = 0.0f;
        mat->roughness          = 0.8f;
        mat->emissive           = 0.0f;
        mat->clearcoat          = 0.0f;
        mat->clearcoat_roughness= 0.5f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.45f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.0f;
        break;

    case JI_MATERIAL_GLASS:
        mat->metallic           = 0.0f;
        mat->roughness          = 0.05f;
        mat->emissive           = 0.0f;
        mat->clearcoat          = 1.0f;
        mat->clearcoat_roughness= 0.03f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.52f;
        mat->blur_amount         = 2.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.0f;
        break;

    case JI_MATERIAL_METAL:
        mat->metallic           = 1.0f;
        mat->roughness          = 0.3f;
        mat->emissive           = 0.0f;
        mat->clearcoat          = 0.0f;
        mat->clearcoat_roughness= 0.5f;
        mat->anisotropy          = 0.5f;
        mat->ior                 = 1.0f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.0f;
        break;

    case JI_MATERIAL_CARBON:
        mat->metallic           = 0.7f;
        mat->roughness          = 0.4f;
        mat->emissive           = 0.0f;
        mat->clearcoat          = 0.5f;
        mat->clearcoat_roughness= 0.1f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.0f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.0f;
        break;

    case JI_MATERIAL_NEON:
        mat->metallic           = 0.0f;
        mat->roughness          = 0.2f;
        mat->emissive           = 2.0f;
        mat->clearcoat          = 0.0f;
        mat->clearcoat_roughness= 0.5f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.0f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.8f;
        mat->holographic_shift   = 0.0f;
        break;

    case JI_MATERIAL_HOLOGRAPHIC:
        mat->metallic           = 0.3f;
        mat->roughness          = 0.1f;
        mat->emissive           = 0.5f;
        mat->clearcoat          = 1.0f;
        mat->clearcoat_roughness= 0.0f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.3f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.5f;
        break;

    case JI_MATERIAL_PBR:
        mat->metallic           = 0.0f;
        mat->roughness          = 0.5f;
        mat->emissive           = 0.0f;
        mat->clearcoat          = 0.0f;
        mat->clearcoat_roughness= 0.5f;
        mat->anisotropy          = 0.0f;
        mat->ior                 = 1.5f;
        mat->blur_amount         = 0.0f;
        mat->bloom_threshold     = 0.0f;
        mat->holographic_shift   = 0.0f;
        break;
    }
}

/* =========================================================================
 * Material Engine — Lifecycle
 * ========================================================================= */

JI_API JiMaterialEngine* ji_material_engine_new(void) {
    JiMaterialEngine* engine = JI_NEW(JiMaterialEngine);
    if (!engine) return NULL;

    engine->material_capacity = 32;
    engine->materials = JI_NEW_ARRAY(JiUIMaterial*, engine->material_capacity);
    if (!engine->materials) {
        ji_free(engine);
        return NULL;
    }
    engine->material_count = 0;
    engine->global_time = 0.0f;
    return engine;
}

JI_API void ji_material_engine_destroy(JiMaterialEngine* engine) {
    if (!engine) return;
    /* Note: does not destroy individual materials — caller owns them */
    if (engine->materials) {
        ji_free(engine->materials);
    }
    ji_free(engine);
}

JI_API void ji_material_engine_update(JiMaterialEngine* engine, float delta_time) {
    if (!engine) return;

    engine->global_time += delta_time;

    for (int i = 0; i < engine->material_count; i++) {
        JiUIMaterial* mat = engine->materials[i];
        if (!mat) continue;

        mat->animation_time += delta_time * mat->animation_speed;

        /* Handle transitions */
        JiMaterialTransition* trans = (JiMaterialTransition*)mat->normal_map;
        if (trans && trans->active) {
            trans->elapsed += delta_time;
            float t = ji_clampf(trans->elapsed / trans->duration, 0.0f, 1.0f);
            float eased = ji_ease_in_out(t);

            /* Interpolate properties */
            JiUIMaterial target;
            memset(&target, 0, sizeof(target));
            target.type = trans->to_type;
            ji_material_apply_defaults(&target);

            mat->base_color = ji_vec4_lerp(trans->from_base_color, target.base_color, eased);
            mat->metallic             = ji_lerpf(trans->from_metallic,             target.metallic,             eased);
            mat->roughness            = ji_lerpf(trans->from_roughness,            target.roughness,            eased);
            mat->emissive             = ji_lerpf(trans->from_emissive,             target.emissive,             eased);
            mat->clearcoat            = ji_lerpf(trans->from_clearcoat,            target.clearcoat,            eased);
            mat->clearcoat_roughness  = ji_lerpf(trans->from_clearcoat_roughness, target.clearcoat_roughness,  eased);
            mat->anisotropy           = ji_lerpf(trans->from_anisotropy,           target.anisotropy,           eased);
            mat->ior                  = ji_lerpf(trans->from_ior,                  target.ior,                  eased);
            mat->blur_amount          = ji_lerpf(trans->from_blur_amount,          target.blur_amount,          eased);
            mat->bloom_threshold      = ji_lerpf(trans->from_bloom_threshold,      target.bloom_threshold,      eased);
            mat->holographic_shift    = ji_lerpf(trans->from_holographic_shift,   target.holographic_shift,    eased);

            if (t >= 1.0f) {
                /* Transition complete */
                mat->type = trans->to_type;
                trans->active = false;
                ji_free(trans);
                mat->normal_map = NULL;
            }
        }
    }
}

/* =========================================================================
 * Material Creation
 * ========================================================================= */

JI_API JiUIMaterial* ji_ui_material_new(JiMaterialType type) {
    JiUIMaterial* mat = JI_NEW(JiUIMaterial);
    if (!mat) return NULL;

    memset(mat, 0, sizeof(JiUIMaterial));
    mat->type = type;
    mat->base_color.x = 1.0f;
    mat->base_color.y = 1.0f;
    mat->base_color.z = 1.0f;
    mat->base_color.w = 1.0f;
    mat->emissive_color.x = 1.0f;
    mat->emissive_color.y = 1.0f;
    mat->emissive_color.z = 1.0f;
    mat->emissive_color.w = 1.0f;
    mat->animation_speed = 1.0f;
    mat->cast_shadows = true;
    mat->receive_shadows = true;
    mat->double_sided = false;
    mat->wireframe = false;

    ji_material_apply_defaults(mat);
    return mat;
}

JI_API void ji_ui_material_destroy(JiUIMaterial* mat) {
    if (!mat) return;

    /* Free transition state if active */
    JiMaterialTransition* trans = (JiMaterialTransition*)mat->normal_map;
    if (trans) {
        ji_free(trans);
        mat->normal_map = NULL;
    }
    ji_free(mat);
}

JI_API JiUIMaterial* ji_material_flat_new(float r, float g, float b, float a) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_FLAT);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = a;
    return mat;
}

JI_API JiUIMaterial* ji_material_glass_new(float r, float g, float b, float a,
                                             float blur, float ior) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_GLASS);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = a;
    mat->blur_amount = blur;
    mat->ior = ior;
    return mat;
}

JI_API JiUIMaterial* ji_material_metal_new(float r, float g, float b,
                                             float roughness, float anisotropy) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_METAL);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = 1.0f;
    mat->roughness = roughness;
    mat->anisotropy = anisotropy;
    return mat;
}

JI_API JiUIMaterial* ji_material_carbon_new(float r, float g, float b) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_CARBON);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = 1.0f;
    return mat;
}

JI_API JiUIMaterial* ji_material_neon_new(float r, float g, float b,
                                            float emissive, float bloom) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_NEON);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = 1.0f;
    mat->emissive_color.x = r;
    mat->emissive_color.y = g;
    mat->emissive_color.z = b;
    mat->emissive_color.w = 1.0f;
    mat->emissive = emissive;
    mat->bloom_threshold = bloom;
    return mat;
}

JI_API JiUIMaterial* ji_material_holographic_new(float shift) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_HOLOGRAPHIC);
    if (!mat) return NULL;
    mat->holographic_shift = shift;
    return mat;
}

JI_API JiUIMaterial* ji_material_pbr_new(float r, float g, float b,
                                            float metallic, float roughness) {
    JiUIMaterial* mat = ji_ui_material_new(JI_MATERIAL_PBR);
    if (!mat) return NULL;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = 1.0f;
    mat->metallic = metallic;
    mat->roughness = roughness;
    return mat;
}

/* =========================================================================
 * Material Properties
 * ========================================================================= */

JI_API void ji_ui_material_set_color(JiUIMaterial* mat, float r, float g, float b, float a) {
    if (!mat) return;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = a;
}

JI_API void ji_ui_material_set_metallic_roughness(JiUIMaterial* mat,
                                                    float metallic, float roughness) {
    if (!mat) return;
    mat->metallic = ji_clampf(metallic, 0.0f, 1.0f);
    mat->roughness = ji_clampf(roughness, 0.0f, 1.0f);
}

JI_API void ji_ui_material_set_emissive(JiUIMaterial* mat,
                                          float r, float g, float b, float intensity) {
    if (!mat) return;
    mat->emissive_color.x = r;
    mat->emissive_color.y = g;
    mat->emissive_color.z = b;
    mat->emissive_color.w = 1.0f;
    mat->emissive = intensity;
}

JI_API void ji_ui_material_set_clearcoat(JiUIMaterial* mat,
                                           float clearcoat, float roughness) {
    if (!mat) return;
    mat->clearcoat = ji_clampf(clearcoat, 0.0f, 1.0f);
    mat->clearcoat_roughness = ji_clampf(roughness, 0.0f, 1.0f);
}

JI_API void ji_ui_material_set_glass(JiUIMaterial* mat, float blur, float ior) {
    if (!mat) return;
    mat->blur_amount = ji_clampf(blur, 0.0f, 20.0f);
    mat->ior = ji_clampf(ior, 1.0f, 3.0f);
}

JI_API void ji_ui_material_set_neon(JiUIMaterial* mat, float bloom_threshold) {
    if (!mat) return;
    mat->bloom_threshold = ji_clampf(bloom_threshold, 0.0f, 1.0f);
}

JI_API void ji_ui_material_set_holographic(JiUIMaterial* mat, float shift) {
    if (!mat) return;
    mat->holographic_shift = shift;
}

JI_API void ji_ui_material_set_animation(JiUIMaterial* mat, float speed) {
    if (!mat) return;
    mat->animation_speed = speed;
}

JI_API void ji_ui_material_set_wireframe(JiUIMaterial* mat, bool wireframe) {
    if (!mat) return;
    mat->wireframe = wireframe;
}

JI_API void ji_ui_material_set_double_sided(JiUIMaterial* mat, bool double_sided) {
    if (!mat) return;
    mat->double_sided = double_sided;
}

JI_API JiMaterialType ji_ui_material_type(const JiUIMaterial* mat) {
    if (!mat) return JI_MATERIAL_FLAT;
    return mat->type;
}

/* =========================================================================
 * Material Transitions
 * ========================================================================= */

JI_API bool ji_material_engine_transition(JiMaterialEngine* engine,
                                             JiUIMaterial* mat,
                                             JiMaterialType new_type,
                                             float duration) {
    (void)engine;
    if (!mat || duration <= 0.0f) return false;

    /* If already transitioning, free old transition */
    JiMaterialTransition* old = (JiMaterialTransition*)mat->normal_map;
    if (old) {
        ji_free(old);
        mat->normal_map = NULL;
    }

    JiMaterialTransition* trans = JI_NEW(JiMaterialTransition);
    if (!trans) return false;

    trans->from_type                = mat->type;
    trans->to_type                  = new_type;
    trans->duration                 = duration;
    trans->elapsed                  = 0.0f;
    trans->active                   = true;
    trans->from_base_color           = mat->base_color;
    trans->from_metallic             = mat->metallic;
    trans->from_roughness            = mat->roughness;
    trans->from_emissive             = mat->emissive;
    trans->from_clearcoat            = mat->clearcoat;
    trans->from_clearcoat_roughness  = mat->clearcoat_roughness;
    trans->from_anisotropy           = mat->anisotropy;
    trans->from_ior                  = mat->ior;
    trans->from_blur_amount          = mat->blur_amount;
    trans->from_bloom_threshold      = mat->bloom_threshold;
    trans->from_holographic_shift    = mat->holographic_shift;

    mat->normal_map = (void*)trans;
    return true;
}

JI_API bool ji_material_is_transitioning(const JiUIMaterial* mat) {
    if (!mat) return false;
    JiMaterialTransition* trans = (JiMaterialTransition*)mat->normal_map;
    return trans && trans->active;
}

/* =========================================================================
 * Material Registration
 * ========================================================================= */

JI_API bool ji_material_engine_add(JiMaterialEngine* engine, JiUIMaterial* mat) {
    if (!engine || !mat) return false;

    /* Check if already registered */
    for (int i = 0; i < engine->material_count; i++) {
        if (engine->materials[i] == mat) return true;
    }

    /* Grow array if needed */
    if (engine->material_count >= engine->material_capacity) {
        int new_cap = engine->material_capacity * 2;
        JiUIMaterial** new_arr = (JiUIMaterial**)ji_realloc(engine->materials,
                              (size_t)new_cap * sizeof(JiUIMaterial*));
        if (!new_arr) return false;
        engine->materials = new_arr;
        engine->material_capacity = new_cap;
    }

    engine->materials[engine->material_count++] = mat;
    return true;
}

JI_API bool ji_material_engine_remove(JiMaterialEngine* engine, JiUIMaterial* mat) {
    if (!engine || !mat) return false;

    for (int i = 0; i < engine->material_count; i++) {
        if (engine->materials[i] == mat) {
            /* Shift remaining elements down */
            for (int j = i; j < engine->material_count - 1; j++) {
                engine->materials[j] = engine->materials[j + 1];
            }
            engine->material_count--;
            return true;
        }
    }
    return false;
}

JI_API int ji_material_engine_count(const JiMaterialEngine* engine) {
    if (!engine) return 0;
    return engine->material_count;
}
