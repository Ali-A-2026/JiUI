/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_material_glass.c
 * @brief Glass material — refraction approximation, blur-through, fresnel
 *        edge tinting, and chromatic aberration simulation.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_3d.h"
#include <math.h>

/* =========================================================================
 * Glass Internal Helpers
 * ========================================================================= */

static float glass_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Schlick's approximation for Fresnel reflectance at a dielectric interface */
static float glass_fresnel(float cos_theta, float ior) {
    float r0 = (1.0f - ior) / (1.0f + ior);
    r0 = r0 * r0;
    float c = glass_clampf(cos_theta, 0.0f, 1.0f);
    return r0 + (1.0f - r0) * powf(1.0f - c, 5.0f);
}

/* =========================================================================
 * Glass Public API
 *
 * These CPU-side evaluation functions are used for software fallback
 * rendering and testing. On GPU, equivalent shaders would be used.
 * ========================================================================= */

/**
 * Compute glass refraction direction using Snell's law.
 *
 * @param incident   Incident ray direction (normalized, pointing towards surface).
 * @param normal     Surface normal (normalized, pointing away from surface).
 * @param ior        Index of refraction ratio (eta = n1/n2).
 * @return           Refracted direction (normalized), or total internal reflection indicator.
 */
JI_API JiVec3 ji_glass_refract(JiVec3 incident, JiVec3 normal, float ior) {
    float cos_i = -(incident.x * normal.x + incident.y * normal.y + incident.z * normal.z);
    cos_i = glass_clampf(cos_i, -1.0f, 1.0f);

    float sin_t2 = ior * ior * (1.0f - cos_i * cos_i);
    if (sin_t2 > 1.0f) {
        /* Total internal reflection — reflect instead */
        JiVec3 refl = {
            incident.x + 2.0f * cos_i * normal.x,
            incident.y + 2.0f * cos_i * normal.y,
            incident.z + 2.0f * cos_i * normal.z
        };
        return refl;
    }

    float cos_t = sqrtf(1.0f - sin_t2);
    JiVec3 refr = {
        ior * incident.x + (ior * cos_i - cos_t) * normal.x,
        ior * incident.y + (ior * cos_i - cos_t) * normal.y,
        ior * incident.z + (ior * cos_i - cos_t) * normal.z
    };
    return refr;
}

/**
 * Evaluate glass material for a given view angle.
 * Returns the blended color (refraction + reflection + edge tint).
 *
 * @param mat          Glass material.
 * @param normal       Surface normal (normalized).
 * @param view_dir     View direction (from surface to camera, normalized).
 * @param backdrop     Color of what's behind the glass (RGBA in JiVec4).
 * @return             Final glass color (RGB in JiVec3).
 */
JI_API JiVec3 ji_glass_evaluate(const JiUIMaterial* mat,
                                  JiVec3 normal, JiVec3 view_dir,
                                  JiVec4 backdrop) {
    if (!mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    float NdotV = glass_clampf(normal.x * view_dir.x + normal.y * view_dir.y + normal.z * view_dir.z, 0.0f, 1.0f);

    /* Fresnel reflectance at this angle */
    float fresnel = glass_fresnel(NdotV, mat->ior);

    /* Base tint from material color */
    JiVec3 tint = {mat->base_color.x, mat->base_color.y, mat->base_color.z};

    /* Refracted (background) contribution — tinted by glass color */
    float refraction_strength = 1.0f - fresnel;
    JiVec3 refr_color = {
        backdrop.x * tint.x * refraction_strength,
        backdrop.y * tint.y * refraction_strength,
        backdrop.z * tint.z * refraction_strength
    };

    /* Reflected (specular) contribution — sky/environment color */
    /* For UI, we use a simple environment approximation */
    JiVec3 env_color = {0.8f, 0.85f, 0.9f}; /* soft sky */
    JiVec3 refl_color = {
        env_color.x * fresnel,
        env_color.y * fresnel,
        env_color.z * fresnel
    };

    /* Edge tint — stronger color at grazing angles */
    float edge_factor = powf(1.0f - NdotV, 3.0f);
    JiVec3 edge_tint = {
        tint.x * edge_factor * 0.5f,
        tint.y * edge_factor * 0.5f,
        tint.z * edge_factor * 0.5f
    };

    /* Combine */
    JiVec3 result = {
        refr_color.x + refl_color.x + edge_tint.x,
        refr_color.y + refl_color.y + edge_tint.y,
        refr_color.z + refl_color.z + edge_tint.z
    };

    /* Apply transparency (alpha blending with backdrop) */
    float alpha = glass_clampf(mat->base_color.w, 0.0f, 1.0f);
    result.x = result.x * alpha + backdrop.x * (1.0f - alpha);
    result.y = result.y * alpha + backdrop.y * (1.0f - alpha);
    result.z = result.z * alpha + backdrop.z * (1.0f - alpha);

    return result;
}

/**
 * Compute chromatic aberration for glass (per-channel IOR offset).
 * Returns three slightly different refraction directions for R, G, B channels.
 *
 * @param incident    Incident ray direction (normalized).
 * @param normal      Surface normal (normalized).
 * @param ior         Base index of refraction.
 * @param aberration  Aberration amount (typically 0.001 - 0.01).
 * @param out_r       Output: refracted direction for red channel.
 * @param out_g       Output: refracted direction for green channel.
 * @param out_b       Output: refracted direction for blue channel.
 */
JI_API void ji_glass_chromatic_aberration(JiVec3 incident, JiVec3 normal,
                                            float ior, float aberration,
                                            JiVec3* out_r, JiVec3* out_g, JiVec3* out_b) {
    if (!out_r || !out_g || !out_b) return;

    /* Red has lower IOR, blue has higher IOR (simplified dispersion) */
    *out_r = ji_glass_refract(incident, normal, ior - aberration);
    *out_g = ji_glass_refract(incident, normal, ior);
    *out_b = ji_glass_refract(incident, normal, ior + aberration);
}

/**
 * Compute blur kernel size for glass based on distance and blur amount.
 *
 * @param mat          Glass material.
 * @param distance     Distance through the glass (in pixels).
 * @return             Blur kernel radius in pixels.
 */
JI_API float ji_glass_blur_radius(const JiUIMaterial* mat, float distance) {
    if (!mat) return 0.0f;
    return mat->blur_amount * glass_clampf(distance, 0.0f, 100.0f) * 0.1f;
}
