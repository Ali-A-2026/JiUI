/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_material.h
 * @brief JiUI Material Engine — PBR UI materials, glass, metal, neon, holographic.
 *
 * Provides advanced material types for UI rendering:
 *   - Flat (standard flat UI)
 *   - Glass (frosted glass with blur-through)
 *   - Metal (brushed metal, anisotropic highlights)
 *   - Carbon (carbon fiber procedural texture)
 *   - Neon (emissive edges + bloom)
 *   - Holographic (iridescent shimmer)
 *   - PBR (full metallic-roughness pipeline)
 */

#ifndef JIUI_MATERIAL_H
#define JIUI_MATERIAL_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include "ji_3d.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Material Types
 * ========================================================================= */

typedef enum JiMaterialType {
    JI_MATERIAL_FLAT = 0,        /* Standard flat UI */
    JI_MATERIAL_GLASS,           /* Frosted glass, blur-through */
    JI_MATERIAL_METAL,           /* Brushed metal, anisotropic */
    JI_MATERIAL_CARBON,          /* Carbon fiber texture */
    JI_MATERIAL_NEON,            /* Neon glow, emissive edges */
    JI_MATERIAL_HOLOGRAPHIC,     /* Holographic shimmer */
    JI_MATERIAL_PBR              /* Full PBR pipeline */
} JiMaterialType;

/* =========================================================================
 * UI Material — extends Ji3DMaterial with UI-specific properties
 * ========================================================================= */

typedef struct JiUIMaterial {
    JiMaterialType  type;
    JiVec4         base_color;       /* RGBA base color */
    float          metallic;         /* 0 = dielectric, 1 = metal */
    float          roughness;        /* 0 = smooth, 1 = rough */
    float          emissive;         /* Emissive intensity */
    JiVec4         emissive_color;   /* Emissive color */
    float          clearcoat;        /* Clearcoat layer intensity */
    float          clearcoat_roughness;
    float          anisotropy;       /* Anisotropic highlight direction */
    float          ior;              /* Index of refraction for glass */
    float          blur_amount;      /* Blur radius for glass */
    float          bloom_threshold;  /* Bloom cutoff for neon */
    float          holographic_shift;/* Iridescence shift for holographic */
    bool           wireframe;
    bool           double_sided;
    bool           cast_shadows;
    bool           receive_shadows;
    /* Texture maps (optional, NULL = not used) */
    void*          normal_map;        /* JiTexture* */
    void*          roughness_map;
    void*          metallic_map;
    void*          emissive_map;
    /* Animation */
    float          animation_time;   /* Current animation time */
    float          animation_speed;  /* Animation speed multiplier */
} JiUIMaterial;

/* =========================================================================
 * Material Engine — manages material instances and transitions
 * ========================================================================= */

typedef struct JiMaterialEngine {
    JiUIMaterial**  materials;
    int             material_count;
    int             material_capacity;
    float           global_time;
} JiMaterialEngine;

/* =========================================================================
 * Material Engine — Lifecycle
 * ========================================================================= */

/** Create a new material engine. */
JI_API JiMaterialEngine* ji_material_engine_new(void);

/** Destroy a material engine. */
JI_API void ji_material_engine_destroy(JiMaterialEngine* engine);

/** Update the engine (advance animations). Call each frame. */
JI_API void ji_material_engine_update(JiMaterialEngine* engine, float delta_time);

/* =========================================================================
 * Material Creation
 * ========================================================================= */

/** Create a new material of the given type. */
JI_API JiUIMaterial* ji_ui_material_new(JiMaterialType type);

/** Destroy a material. */
JI_API void ji_ui_material_destroy(JiUIMaterial* mat);

/** Create a flat material. */
JI_API JiUIMaterial* ji_material_flat_new(float r, float g, float b, float a);

/** Create a glass material. */
JI_API JiUIMaterial* ji_material_glass_new(float r, float g, float b, float a,
                                             float blur, float ior);

/** Create a metal material. */
JI_API JiUIMaterial* ji_material_metal_new(float r, float g, float b,
                                             float roughness, float anisotropy);

/** Create a carbon fiber material. */
JI_API JiUIMaterial* ji_material_carbon_new(float r, float g, float b);

/** Create a neon material. */
JI_API JiUIMaterial* ji_material_neon_new(float r, float g, float b,
                                            float emissive, float bloom);

/** Create a holographic material. */
JI_API JiUIMaterial* ji_material_holographic_new(float shift);

/** Create a PBR material. */
JI_API JiUIMaterial* ji_material_pbr_new(float r, float g, float b,
                                            float metallic, float roughness);

/* =========================================================================
 * Material Properties
 * ========================================================================= */

/** Set base color. */
JI_API void ji_ui_material_set_color(JiUIMaterial* mat, float r, float g, float b, float a);

/** Set metallic/roughness. */
JI_API void ji_ui_material_set_metallic_roughness(JiUIMaterial* mat,
                                                    float metallic, float roughness);

/** Set emissive properties. */
JI_API void ji_ui_material_set_emissive(JiUIMaterial* mat,
                                          float r, float g, float b, float intensity);

/** Set clearcoat. */
JI_API void ji_ui_material_set_clearcoat(JiUIMaterial* mat,
                                           float clearcoat, float roughness);

/** Set glass properties (blur, IOR). */
JI_API void ji_ui_material_set_glass(JiUIMaterial* mat, float blur, float ior);

/** Set neon properties (bloom threshold). */
JI_API void ji_ui_material_set_neon(JiUIMaterial* mat, float bloom_threshold);

/** Set holographic shift. */
JI_API void ji_ui_material_set_holographic(JiUIMaterial* mat, float shift);

/** Set animation speed. */
JI_API void ji_ui_material_set_animation(JiUIMaterial* mat, float speed);

/** Set wireframe mode. */
JI_API void ji_ui_material_set_wireframe(JiUIMaterial* mat, bool wireframe);

/** Set double-sided rendering. */
JI_API void ji_ui_material_set_double_sided(JiUIMaterial* mat, bool double_sided);

/** Get material type. */
JI_API JiMaterialType ji_ui_material_type(const JiUIMaterial* mat);

/* =========================================================================
 * Material Transitions
 * ========================================================================= */

/** Transition a material to a new type over a duration. */
JI_API bool ji_material_engine_transition(JiMaterialEngine* engine,
                                             JiUIMaterial* mat,
                                             JiMaterialType new_type,
                                             float duration);

/** Check if a transition is in progress. */
JI_API bool ji_material_is_transitioning(const JiUIMaterial* mat);

/* =========================================================================
 * Material Registration
 * ========================================================================= */

/** Register a material with the engine. */
JI_API bool ji_material_engine_add(JiMaterialEngine* engine, JiUIMaterial* mat);

/** Remove a material from the engine. */
JI_API bool ji_material_engine_remove(JiMaterialEngine* engine, JiUIMaterial* mat);

/** Get material count. */
JI_API int ji_material_engine_count(const JiMaterialEngine* engine);

/* =========================================================================
 * PBR Pipeline — Cook-Torrance BRDF evaluation (CPU-side, software fallback)
 * ========================================================================= */

/** Evaluate PBR for a single point under one light. */
JI_API JiVec3 ji_pbr_evaluate(const JiUIMaterial* mat,
                                JiVec3 normal, JiVec3 view_dir,
                                JiVec3 light_dir,
                                JiVec3 light_color, float light_intensity);

/** Evaluate ambient/IBL term (hemispheric ambient approximation). */
JI_API JiVec3 ji_pbr_evaluate_ambient(const JiUIMaterial* mat,
                                        JiVec3 normal,
                                        JiVec3 sky_color,
                                        JiVec3 ground_color);

/** Evaluate clearcoat layer (separate specular lobe on top of base). */
JI_API JiVec3 ji_pbr_evaluate_clearcoat(const JiUIMaterial* mat,
                                          JiVec3 normal, JiVec3 view_dir,
                                          JiVec3 light_dir,
                                          JiVec3 light_color, float light_intensity);

/* =========================================================================
 * Glass Material — refraction, blur, chromatic aberration
 * ========================================================================= */

/** Compute glass refraction direction using Snell's law. */
JI_API JiVec3 ji_glass_refract(JiVec3 incident, JiVec3 normal, float ior);

/** Evaluate glass material for a given view angle and backdrop. */
JI_API JiVec3 ji_glass_evaluate(const JiUIMaterial* mat,
                                  JiVec3 normal, JiVec3 view_dir,
                                  JiVec4 backdrop);

/** Compute chromatic aberration (per-channel refraction directions). */
JI_API void ji_glass_chromatic_aberration(JiVec3 incident, JiVec3 normal,
                                            float ior, float aberration,
                                            JiVec3* out_r, JiVec3* out_g, JiVec3* out_b);

/** Compute blur kernel radius for glass based on distance. */
JI_API float ji_glass_blur_radius(const JiUIMaterial* mat, float distance);

/* =========================================================================
 * Neon Material — emissive edges, bloom, glow
 * ========================================================================= */

/** Evaluate neon glow at a given distance from the edge. */
JI_API float ji_neon_glow_at(const JiUIMaterial* mat, float edge_distance);

/** Evaluate neon color at a point, including pulse animation. */
JI_API JiVec3 ji_neon_evaluate(const JiUIMaterial* mat, float edge_distance);

/** Compute bloom contribution for a neon material. */
JI_API JiVec3 ji_neon_bloom_contribution(const JiUIMaterial* mat, JiVec3 base_color);

/** Compute neon edge intensity from a distance field value. */
JI_API float ji_neon_edge_intensity(float distance_field, float edge_width);

/* =========================================================================
 * Dynamic Lighting System
 * ========================================================================= */

typedef struct JiLightingSystem JiLightingSystem;

/** Create a new lighting system. */
JI_API JiLightingSystem* ji_lighting_new(void);

/** Destroy a lighting system. */
JI_API void ji_lighting_destroy(JiLightingSystem* sys);

/** Set global ambient color and intensity. */
JI_API void ji_lighting_set_ambient(JiLightingSystem* sys,
                                       float r, float g, float b, float intensity);

/** Add a light to the system. */
JI_API bool ji_lighting_add(JiLightingSystem* sys, Ji3DLight* light);

/** Remove a light from the system. */
JI_API bool ji_lighting_remove(JiLightingSystem* sys, Ji3DLight* light);

/** Get light count. */
JI_API int ji_lighting_count(const JiLightingSystem* sys);

/** Update the lighting system (advance time). */
JI_API void ji_lighting_update(JiLightingSystem* sys, float delta_time);

/** Compute light direction and intensity at a world position. */
JI_API void ji_lighting_compute(const Ji3DLight* light,
                                   JiVec3 world_pos,
                                   JiVec3* out_dir,
                                   float* out_intensity);

/** Evaluate full lighting for a material at a 3D position. */
JI_API JiVec3 ji_lighting_evaluate(const JiLightingSystem* sys,
                                     const JiUIMaterial* mat,
                                     JiVec3 world_pos,
                                     JiVec3 normal, JiVec3 view_dir);

/** Evaluate lighting for a 2D UI element (simplified). */
JI_API JiVec3 ji_lighting_evaluate_2d(const JiLightingSystem* sys,
                                        const JiUIMaterial* mat,
                                        JiVec3 light_dir);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_MATERIAL_H */
