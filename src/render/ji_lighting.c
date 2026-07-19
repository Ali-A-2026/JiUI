/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_lighting.c
 * @brief Dynamic lighting system — manages multiple light sources, computes
 *        per-pixel lighting for materials, supports directional, point, spot,
 *        and ambient lights with attenuation and shadow flags.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * Lighting System
 * ========================================================================= */

#define JI_MAX_LIGHTS 16

typedef struct JiLightingSystem {
    Ji3DLight* lights[JI_MAX_LIGHTS];
    int        light_count;
    JiVec3     ambient_color;     /* Global ambient color */
    float      ambient_intensity; /* Global ambient intensity */
    float      global_time;
} JiLightingSystem;

/* =========================================================================
 * Helpers
 * ========================================================================= */

static float lighting_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float lighting_attenuation(float distance, float range) {
    if (range <= 0.0f) return 1.0f;
    float d = distance / range;
    /* Smooth inverse-square-like falloff */
    return 1.0f / (1.0f + d * d * d * d);
}

static float lighting_spot_factor(JiVec3 light_dir, JiVec3 spot_dir,
                                     float inner_angle, float outer_angle) {
    float cos_theta = -(light_dir.x * spot_dir.x + light_dir.y * spot_dir.y + light_dir.z * spot_dir.z);
    float inner_cos = cosf(inner_angle * (float)(M_PI / 180.0) * 0.5f);
    float outer_cos = cosf(outer_angle * (float)(M_PI / 180.0) * 0.5f);
    if (cos_theta < outer_cos) return 0.0f;
    if (cos_theta > inner_cos) return 1.0f;
    return (cos_theta - outer_cos) / (inner_cos - outer_cos);
}

/* =========================================================================
 * Lighting System — Lifecycle
 * ========================================================================= */

JI_API JiLightingSystem* ji_lighting_new(void) {
    JiLightingSystem* sys = JI_NEW(JiLightingSystem);
    if (!sys) return NULL;
    memset(sys, 0, sizeof(JiLightingSystem));
    sys->ambient_color.x = 0.2f;
    sys->ambient_color.y = 0.2f;
    sys->ambient_color.z = 0.25f;
    sys->ambient_intensity = 0.3f;
    return sys;
}

JI_API void ji_lighting_destroy(JiLightingSystem* sys) {
    if (!sys) return;
    /* Lights are not owned by the system — caller manages them */
    ji_free(sys);
}

JI_API void ji_lighting_set_ambient(JiLightingSystem* sys,
                                       float r, float g, float b, float intensity) {
    if (!sys) return;
    sys->ambient_color.x = r;
    sys->ambient_color.y = g;
    sys->ambient_color.z = b;
    sys->ambient_intensity = lighting_clampf(intensity, 0.0f, 2.0f);
}

JI_API bool ji_lighting_add(JiLightingSystem* sys, Ji3DLight* light) {
    if (!sys || !light) return false;
    if (sys->light_count >= JI_MAX_LIGHTS) return false;
    /* Check for duplicates */
    for (int i = 0; i < sys->light_count; i++) {
        if (sys->lights[i] == light) return true;
    }
    sys->lights[sys->light_count++] = light;
    return true;
}

JI_API bool ji_lighting_remove(JiLightingSystem* sys, Ji3DLight* light) {
    if (!sys || !light) return false;
    for (int i = 0; i < sys->light_count; i++) {
        if (sys->lights[i] == light) {
            for (int j = i; j < sys->light_count - 1; j++) {
                sys->lights[j] = sys->lights[j + 1];
            }
            sys->light_count--;
            return true;
        }
    }
    return false;
}

JI_API int ji_lighting_count(const JiLightingSystem* sys) {
    return sys ? sys->light_count : 0;
}

JI_API void ji_lighting_update(JiLightingSystem* sys, float delta_time) {
    if (!sys) return;
    sys->global_time += delta_time;
}

/* =========================================================================
 * Lighting Evaluation
 * ========================================================================= */

/**
 * Compute the light direction and intensity at a given world position
 * for a specific light source.
 */
JI_API void ji_lighting_compute(const Ji3DLight* light,
                                   JiVec3 world_pos,
                                   JiVec3* out_dir,
                                   float* out_intensity) {
    if (!light || !out_dir || !out_intensity) return;

    JiVec3 light_color = {light->color.x, light->color.y, light->color.z};
    float base_intensity = light->color.w;

    switch (light->type) {
    case JI_3D_LIGHT_DIRECTIONAL: {
        /* Directional light: direction is constant */
        *out_dir = light->direction;
        /* Normalize */
        float len = sqrtf(out_dir->x * out_dir->x + out_dir->y * out_dir->y + out_dir->z * out_dir->z);
        if (len > 0.0001f) {
            out_dir->x /= len; out_dir->y /= len; out_dir->z /= len;
        }
        *out_intensity = base_intensity;
        break;
    }

    case JI_3D_LIGHT_POINT: {
        /* Point light: direction from position to light */
        JiVec3 to_light = {
            light->position.x - world_pos.x,
            light->position.y - world_pos.y,
            light->position.z - world_pos.z
        };
        float dist = sqrtf(to_light.x * to_light.x + to_light.y * to_light.y + to_light.z * to_light.z);
        if (dist > 0.0001f) {
            out_dir->x = to_light.x / dist;
            out_dir->y = to_light.y / dist;
            out_dir->z = to_light.z / dist;
        } else {
            *out_dir = (JiVec3){0, 1, 0};
        }
        float atten = lighting_attenuation(dist, light->range);
        *out_intensity = base_intensity * atten;
        break;
    }

    case JI_3D_LIGHT_SPOT: {
        /* Spot light: point light with cone */
        JiVec3 to_light = {
            light->position.x - world_pos.x,
            light->position.y - world_pos.y,
            light->position.z - world_pos.z
        };
        float dist = sqrtf(to_light.x * to_light.x + to_light.y * to_light.y + to_light.z * to_light.z);
        if (dist > 0.0001f) {
            out_dir->x = to_light.x / dist;
            out_dir->y = to_light.y / dist;
            out_dir->z = to_light.z / dist;
        } else {
            *out_dir = (JiVec3){0, 1, 0};
        }
        float atten = lighting_attenuation(dist, light->range);
        float spot = lighting_spot_factor(*out_dir, light->direction,
                                            light->inner_angle, light->outer_angle);
        *out_intensity = base_intensity * atten * spot;
        break;
    }

    case JI_3D_LIGHT_AMBIENT: {
        *out_dir = (JiVec3){0, 1, 0};
        *out_intensity = base_intensity;
        break;
    }
    }
}

/**
 * Evaluate full lighting for a material at a given position.
 * Sums contributions from all lights in the system.
 *
 * @param sys        Lighting system.
 * @param mat        Material to light.
 * @param world_pos  World position of the fragment.
 * @param normal     Surface normal (normalized).
 * @param view_dir   View direction (from surface to camera, normalized).
 * @return           Final lit color (RGB in JiVec3).
 */
JI_API JiVec3 ji_lighting_evaluate(const JiLightingSystem* sys,
                                     const JiUIMaterial* mat,
                                     JiVec3 world_pos,
                                     JiVec3 normal, JiVec3 view_dir) {
    if (!sys || !mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    JiVec3 result = {0, 0, 0};

    /* Ambient term */
    JiVec3 sky = {sys->ambient_color.x, sys->ambient_color.y, sys->ambient_color.z};
    JiVec3 ground = {0.15f, 0.12f, 0.1f};
    JiVec3 ambient = ji_pbr_evaluate_ambient(mat, normal, sky, ground);
    result.x += ambient.x * sys->ambient_intensity;
    result.y += ambient.y * sys->ambient_intensity;
    result.z += ambient.z * sys->ambient_intensity;

    /* Per-light contributions */
    for (int i = 0; i < sys->light_count; i++) {
        Ji3DLight* light = sys->lights[i];
        if (!light) continue;

        JiVec3 light_dir;
        float intensity;
        ji_lighting_compute(light, world_pos, &light_dir, &intensity);

        if (intensity <= 0.001f) continue;

        JiVec3 light_color = {light->color.x, light->color.y, light->color.z};

        /* Direct PBR lighting */
        JiVec3 direct = ji_pbr_evaluate(mat, normal, view_dir, light_dir,
                                           light_color, intensity);
        result.x += direct.x;
        result.y += direct.y;
        result.z += direct.z;

        /* Clearcoat contribution */
        if (mat->clearcoat > 0.0f) {
            JiVec3 cc = ji_pbr_evaluate_clearcoat(mat, normal, view_dir,
                                                    light_dir, light_color, intensity);
            result.x += cc.x;
            result.y += cc.y;
            result.z += cc.z;
        }
    }

    return result;
}

/**
 * Evaluate lighting for a 2D UI element (simplified — no 3D position).
 * Uses a fixed normal facing the camera and a configurable light direction.
 *
 * @param sys        Lighting system.
 * @param mat        Material to light.
 * @param light_dir  2D light direction (x, y in screen space, z=0).
 * @return           Final lit color (RGB in JiVec3).
 */
JI_API JiVec3 ji_lighting_evaluate_2d(const JiLightingSystem* sys,
                                        const JiUIMaterial* mat,
                                        JiVec3 light_dir) {
    if (!sys || !mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    /* For 2D UI, normal faces the camera (0, 0, 1) */
    JiVec3 normal = {0.0f, 0.0f, 1.0f};
    JiVec3 view_dir = {0.0f, 0.0f, 1.0f};

    /* Normalize light direction */
    float len = sqrtf(light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z);
    if (len > 0.0001f) {
        light_dir.x /= len;
        light_dir.y /= len;
        light_dir.z /= len;
    }

    JiVec3 result = {0, 0, 0};

    /* Ambient */
    JiVec3 sky = {sys->ambient_color.x, sys->ambient_color.y, sys->ambient_color.z};
    JiVec3 ambient = ji_pbr_evaluate_ambient(mat, normal, sky, sky);
    result.x += ambient.x * sys->ambient_intensity;
    result.y += ambient.y * sys->ambient_intensity;
    result.z += ambient.z * sys->ambient_intensity;

    /* Direct lighting from all lights */
    for (int i = 0; i < sys->light_count; i++) {
        Ji3DLight* light = sys->lights[i];
        if (!light) continue;

        JiVec3 ldir;
        float intensity;
        ji_lighting_compute(light, (JiVec3){0, 0, 0}, &ldir, &intensity);
        if (intensity <= 0.001f) continue;

        JiVec3 light_color = {light->color.x, light->color.y, light->color.z};
        JiVec3 direct = ji_pbr_evaluate(mat, normal, view_dir, ldir,
                                           light_color, intensity);
        result.x += direct.x;
        result.y += direct.y;
        result.z += direct.z;
    }

    return result;
}
