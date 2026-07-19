/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_material_pbr.c
 * @brief PBR (Physically Based Rendering) pipeline — Cook-Torrance BRDF,
 *        metallic-roughness workflow, IBL (image-based lighting) evaluation.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * PBR Internal Helpers — Cook-Torrance BRDF
 * ========================================================================= */

/* Fresnel-Schlick approximation */
static float pbr_fresnel_schlick(float cos_theta, float f0) {
    float clamped = cos_theta < 0.0f ? 0.0f : (cos_theta > 1.0f ? 1.0f : cos_theta);
    return f0 + (1.0f - f0) * powf(1.0f - clamped, 5.0f);
}

/* GGX/Trowbridge-Reitz normal distribution function */
static float pbr_distribution_ggx(float NdotH, float roughness) {
    if (NdotH <= 0.0f) return 0.0f;
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
    return a2 / ((float)M_PI * denom * denom);
}

/* Schlick-GGX geometry function (Smith's method with GGX) */
static float pbr_geometry_schlick_ggx(float NdotV, float roughness) {
    float clamped = NdotV < 0.0f ? 0.0f : (NdotV > 1.0f ? 1.0f : NdotV);
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return clamped / (clamped * (1.0f - k) + k);
}

/* Smith's geometry shadowing (combines view + light directions) */
static float pbr_geometry_smith(float NdotV, float NdotL, float roughness) {
    return pbr_geometry_schlick_ggx(NdotV, roughness) *
           pbr_geometry_schlick_ggx(NdotL, roughness);
}

/* Clamp helper */
static float pbr_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* =========================================================================
 * PBR Public API — Extended functions
 *
 * These are internal evaluation functions used by the renderer to compute
 * per-pixel lighting. They operate on CPU-side data for testing and
 * software fallback rendering.
 * ========================================================================= */

/**
 * Evaluate PBR for a single point under one light.
 *
 * @param mat          Material (must be PBR or compatible).
 * @param normal       Surface normal (normalized).
 * @param view_dir     View direction (from surface to camera, normalized).
 * @param light_dir    Light direction (from surface to light, normalized).
 * @param light_color  Light color (RGB, 0..1).
 * @param light_intensity  Light intensity multiplier.
 * @return             Reflected radiance (RGB in JiVec3).
 */
JI_API JiVec3 ji_pbr_evaluate(const JiUIMaterial* mat,
                                JiVec3 normal, JiVec3 view_dir,
                                JiVec3 light_dir,
                                JiVec3 light_color, float light_intensity) {
    if (!mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    /* Half-vector */
    JiVec3 H = {
        view_dir.x + light_dir.x,
        view_dir.y + light_dir.y,
        view_dir.z + light_dir.z
    };
    float h_len = sqrtf(H.x * H.x + H.y * H.y + H.z * H.z);
    if (h_len > 0.0001f) {
        H.x /= h_len; H.y /= h_len; H.z /= h_len;
    }

    float NdotL = pbr_clampf(normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z, 0.0f, 1.0f);
    float NdotV = pbr_clampf(normal.x * view_dir.x + normal.y * view_dir.y + normal.z * view_dir.z, 0.0f, 1.0f);
    float NdotH = pbr_clampf(normal.x * H.x + normal.y * H.y + normal.z * H.z, 0.0f, 1.0f);
    float VdotH = pbr_clampf(view_dir.x * H.x + view_dir.y * H.y + view_dir.z * H.z, 0.0f, 1.0f);

    /* F0 — reflectance at normal incidence */
    /* Dielectric F0 = 0.04, metallic F0 = base color */
    float f0_dielectric = 0.04f;
    JiVec3 f0 = {
        f0_dielectric * (1.0f - mat->metallic) + mat->base_color.x * mat->metallic,
        f0_dielectric * (1.0f - mat->metallic) + mat->base_color.y * mat->metallic,
        f0_dielectric * (1.0f - mat->metallic) + mat->base_color.z * mat->metallic
    };

    /* Fresnel */
    float fresnel = pbr_fresnel_schlick(VdotH, f0_dielectric);
    JiVec3 F = {
        f0.x + (1.0f - f0.x) * powf(1.0f - VdotH, 5.0f),
        f0.y + (1.0f - f0.y) * powf(1.0f - VdotH, 5.0f),
        f0.z + (1.0f - f0.z) * powf(1.0f - VdotH, 5.0f)
    };

    /* Normal distribution (GGX) */
    float NDF = pbr_distribution_ggx(NdotH, mat->roughness);

    /* Geometry (Smith) */
    float G = pbr_geometry_smith(NdotV, NdotL, mat->roughness);

    /* Specular BRDF — Cook-Torrance */
    float denom = 4.0f * NdotV * NdotL + 0.001f;
    JiVec3 specular = {
        (NDF * G * F.x) / denom,
        (NDF * G * F.y) / denom,
        (NDF * G * F.z) / denom
    };

    /* Diffuse — Lambertian with energy conservation */
    /* kD = (1 - F) * (1 - metallic) */
    JiVec3 kD = {
        (1.0f - F.x) * (1.0f - mat->metallic),
        (1.0f - F.y) * (1.0f - mat->metallic),
        (1.0f - F.z) * (1.0f - mat->metallic)
    };
    float inv_pi = 1.0f / (float)M_PI;
    JiVec3 diffuse = {
        kD.x * mat->base_color.x * inv_pi,
        kD.y * mat->base_color.y * inv_pi,
        kD.z * mat->base_color.z * inv_pi
    };

    /* Combine with light */
    JiVec3 result = {
        (diffuse.x + specular.x) * light_color.x * light_intensity * NdotL,
        (diffuse.y + specular.y) * light_color.y * light_intensity * NdotL,
        (diffuse.z + specular.z) * light_color.z * light_intensity * NdotL
    };

    /* Add emissive contribution */
    if (mat->emissive > 0.0f) {
        result.x += mat->emissive_color.x * mat->emissive;
        result.y += mat->emissive_color.y * mat->emissive;
        result.z += mat->emissive_color.z * mat->emissive;
    }

    return result;
}

/**
 * Evaluate ambient/IBL term (image-based lighting approximation).
 * Uses a simple hemispheric ambient for the software fallback.
 */
JI_API JiVec3 ji_pbr_evaluate_ambient(const JiUIMaterial* mat,
                                        JiVec3 normal,
                                        JiVec3 sky_color,
                                        JiVec3 ground_color) {
    if (!mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    float up = pbr_clampf(normal.y, 0.0f, 1.0f);
    float down = 1.0f - up;

    /* Ambient diffuse */
    JiVec3 ambient = {
        (sky_color.x * up + ground_color.x * down) * mat->base_color.x * 0.3f,
        (sky_color.y * up + ground_color.y * down) * mat->base_color.y * 0.3f,
        (sky_color.z * up + ground_color.z * down) * mat->base_color.z * 0.3f
    };

    /* Simple specular ambient for metals */
    float spec_strength = mat->metallic * 0.5f * (1.0f - mat->roughness);
    ambient.x += sky_color.x * spec_strength;
    ambient.y += sky_color.y * spec_strength;
    ambient.z += sky_color.z * spec_strength;

    return ambient;
}

/**
 * Evaluate clearcoat layer (separate specular lobe on top of base).
 */
JI_API JiVec3 ji_pbr_evaluate_clearcoat(const JiUIMaterial* mat,
                                          JiVec3 normal, JiVec3 view_dir,
                                          JiVec3 light_dir,
                                          JiVec3 light_color, float light_intensity) {
    if (!mat || mat->clearcoat <= 0.0f) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    JiVec3 H = {
        view_dir.x + light_dir.x,
        view_dir.y + light_dir.y,
        view_dir.z + light_dir.z
    };
    float h_len = sqrtf(H.x * H.x + H.y * H.y + H.z * H.z);
    if (h_len > 0.0001f) {
        H.x /= h_len; H.y /= h_len; H.z /= h_len;
    }

    float NdotH = pbr_clampf(normal.x * H.x + normal.y * H.y + normal.z * H.z, 0.0f, 1.0f);
    float NdotL = pbr_clampf(normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z, 0.0f, 1.0f);
    float NdotV = pbr_clampf(normal.x * view_dir.x + normal.y * view_dir.y + normal.z * view_dir.z, 0.0f, 1.0f);

    /* Clearcoat uses a fixed F0 of 0.04 (dielectric) */
    float NDF = pbr_distribution_ggx(NdotH, mat->clearcoat_roughness);
    float G = pbr_geometry_smith(NdotV, NdotL, mat->clearcoat_roughness);
    float F = pbr_fresnel_schlick(NdotV, 0.04f);

    float spec = (NDF * G * F) / (4.0f * NdotV * NdotL + 0.001f);

    JiVec3 result = {
        spec * light_color.x * light_intensity * NdotL * mat->clearcoat,
        spec * light_color.y * light_intensity * NdotL * mat->clearcoat,
        spec * light_color.z * light_intensity * NdotL * mat->clearcoat
    };
    return result;
}
