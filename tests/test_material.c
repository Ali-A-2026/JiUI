/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_material.c
 * @brief Tests for the JiUI Material Engine — material creation, properties,
 *        transitions, PBR evaluation, glass, neon, and lighting.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/* =========================================================================
 * Test helpers
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  [RUN] %s\n", #name); \
        g_tests_run++; \
        name(); \
        g_tests_passed++; \
        printf("  [PASS] %s\n", #name); \
    } while (0)

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "    ASSERT FAILED: %s (line %d)\n", #cond, __LINE__); \
            assert(cond); \
        } \
    } while (0)

#define ASSERT_FLOAT_NEAR(a, b, eps) \
    do { \
        if (fabsf((a) - (b)) > (eps)) { \
            fprintf(stderr, "    ASSERT FAILED: %f != %f (line %d)\n", (a), (b), __LINE__); \
            assert(0); \
        } \
    } while (0)

/* =========================================================================
 * Material creation tests
 * ========================================================================= */

static void test_material_flat_new(void) {
    JiUIMaterial* mat = ji_material_flat_new(0.8f, 0.2f, 0.2f, 1.0f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_FLAT);
    ASSERT_FLOAT_NEAR(mat->base_color.x, 0.8f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.y, 0.2f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.z, 0.2f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.w, 1.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->metallic, 0.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->roughness, 0.8f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_glass_new(void) {
    JiUIMaterial* mat = ji_material_glass_new(0.9f, 0.95f, 1.0f, 0.5f, 3.0f, 1.52f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_GLASS);
    ASSERT_FLOAT_NEAR(mat->blur_amount, 3.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->ior, 1.52f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->clearcoat, 1.0f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_metal_new(void) {
    JiUIMaterial* mat = ji_material_metal_new(0.8f, 0.8f, 0.85f, 0.3f, 0.5f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_METAL);
    ASSERT_FLOAT_NEAR(mat->metallic, 1.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->roughness, 0.3f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->anisotropy, 0.5f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_neon_new(void) {
    JiUIMaterial* mat = ji_material_neon_new(0.0f, 1.0f, 0.2f, 2.0f, 0.8f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_NEON);
    ASSERT_FLOAT_NEAR(mat->emissive, 2.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->bloom_threshold, 0.8f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->emissive_color.y, 1.0f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_holographic_new(void) {
    JiUIMaterial* mat = ji_material_holographic_new(0.7f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_HOLOGRAPHIC);
    ASSERT_FLOAT_NEAR(mat->holographic_shift, 0.7f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_pbr_new(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.6f, 0.7f, 0.8f, 0.2f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_PBR);
    ASSERT_FLOAT_NEAR(mat->metallic, 0.8f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->roughness, 0.2f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_carbon_new(void) {
    JiUIMaterial* mat = ji_material_carbon_new(0.1f, 0.1f, 0.1f);
    ASSERT_TRUE(mat != NULL);
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_CARBON);
    ASSERT_FLOAT_NEAR(mat->metallic, 0.7f, 0.001f);
    ji_ui_material_destroy(mat);
}

/* =========================================================================
 * Material property tests
 * ========================================================================= */

static void test_material_set_color(void) {
    JiUIMaterial* mat = ji_material_flat_new(0, 0, 0, 1);
    ji_ui_material_set_color(mat, 0.1f, 0.2f, 0.3f, 0.4f);
    ASSERT_FLOAT_NEAR(mat->base_color.x, 0.1f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.y, 0.2f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.z, 0.3f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->base_color.w, 0.4f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_set_metallic_roughness(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.5f, 0.5f, 0.0f, 1.0f);
    ji_ui_material_set_metallic_roughness(mat, 0.7f, 0.3f);
    ASSERT_FLOAT_NEAR(mat->metallic, 0.7f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->roughness, 0.3f, 0.001f);
    /* Test clamping */
    ji_ui_material_set_metallic_roughness(mat, -1.0f, 2.0f);
    ASSERT_FLOAT_NEAR(mat->metallic, 0.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->roughness, 1.0f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_set_emissive(void) {
    JiUIMaterial* mat = ji_material_neon_new(1, 0, 0, 1, 0.5f);
    ji_ui_material_set_emissive(mat, 0.1f, 0.2f, 0.3f, 5.0f);
    ASSERT_FLOAT_NEAR(mat->emissive_color.x, 0.1f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->emissive_color.y, 0.2f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->emissive_color.z, 0.3f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->emissive, 5.0f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_set_clearcoat(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.5f, 0.5f, 0.0f, 0.5f);
    ji_ui_material_set_clearcoat(mat, 0.8f, 0.1f);
    ASSERT_FLOAT_NEAR(mat->clearcoat, 0.8f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->clearcoat_roughness, 0.1f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_set_glass(void) {
    JiUIMaterial* mat = ji_material_glass_new(0.9f, 0.95f, 1.0f, 0.5f, 2.0f, 1.5f);
    ji_ui_material_set_glass(mat, 5.0f, 2.0f);
    ASSERT_FLOAT_NEAR(mat->blur_amount, 5.0f, 0.001f);
    ASSERT_FLOAT_NEAR(mat->ior, 2.0f, 0.001f);
    ji_ui_material_destroy(mat);
}

static void test_material_set_wireframe(void) {
    JiUIMaterial* mat = ji_material_flat_new(1, 1, 1, 1);
    ji_ui_material_set_wireframe(mat, true);
    ASSERT_TRUE(mat->wireframe == true);
    ji_ui_material_set_wireframe(mat, false);
    ASSERT_TRUE(mat->wireframe == false);
    ji_ui_material_destroy(mat);
}

static void test_material_set_double_sided(void) {
    JiUIMaterial* mat = ji_material_flat_new(1, 1, 1, 1);
    ji_ui_material_set_double_sided(mat, true);
    ASSERT_TRUE(mat->double_sided == true);
    ji_ui_material_destroy(mat);
}

static void test_material_set_animation(void) {
    JiUIMaterial* mat = ji_material_neon_new(1, 0, 0, 1, 0.5f);
    ji_ui_material_set_animation(mat, 2.5f);
    ASSERT_FLOAT_NEAR(mat->animation_speed, 2.5f, 0.001f);
    ji_ui_material_destroy(mat);
}

/* =========================================================================
 * Material engine tests
 * ========================================================================= */

static void test_material_engine_lifecycle(void) {
    JiMaterialEngine* engine = ji_material_engine_new();
    ASSERT_TRUE(engine != NULL);
    ASSERT_TRUE(ji_material_engine_count(engine) == 0);
    ji_material_engine_destroy(engine);
}

static void test_material_engine_add_remove(void) {
    JiMaterialEngine* engine = ji_material_engine_new();
    ASSERT_TRUE(engine != NULL);

    JiUIMaterial* mat1 = ji_material_flat_new(1, 0, 0, 1);
    JiUIMaterial* mat2 = ji_material_glass_new(0, 1, 0, 0.5f, 2, 1.5f);

    ASSERT_TRUE(ji_material_engine_add(engine, mat1));
    ASSERT_TRUE(ji_material_engine_add(engine, mat2));
    ASSERT_TRUE(ji_material_engine_count(engine) == 2);

    /* Adding same material again should not duplicate */
    ASSERT_TRUE(ji_material_engine_add(engine, mat1));
    ASSERT_TRUE(ji_material_engine_count(engine) == 2);

    ASSERT_TRUE(ji_material_engine_remove(engine, mat1));
    ASSERT_TRUE(ji_material_engine_count(engine) == 1);

    ASSERT_TRUE(ji_material_engine_remove(engine, mat2));
    ASSERT_TRUE(ji_material_engine_count(engine) == 0);

    /* Removing non-existent should fail */
    ASSERT_TRUE(!ji_material_engine_remove(engine, mat1));

    ji_ui_material_destroy(mat1);
    ji_ui_material_destroy(mat2);
    ji_material_engine_destroy(engine);
}

static void test_material_engine_update(void) {
    JiMaterialEngine* engine = ji_material_engine_new();
    JiUIMaterial* mat = ji_material_neon_new(1, 0, 0, 1, 0.5f);
    ji_material_engine_add(engine, mat);

    float initial_time = mat->animation_time;
    ji_material_engine_update(engine, 0.016f);
    ASSERT_TRUE(mat->animation_time > initial_time);

    ji_ui_material_destroy(mat);
    ji_material_engine_destroy(engine);
}

static void test_material_engine_grow(void) {
    JiMaterialEngine* engine = ji_material_engine_new();
    ASSERT_TRUE(engine != NULL);

    /* Add more than initial capacity (32) */
    JiUIMaterial* mats[40];
    for (int i = 0; i < 40; i++) {
        mats[i] = ji_material_flat_new(1, 1, 1, 1);
        ASSERT_TRUE(ji_material_engine_add(engine, mats[i]));
    }
    ASSERT_TRUE(ji_material_engine_count(engine) == 40);

    for (int i = 0; i < 40; i++) {
        ji_ui_material_destroy(mats[i]);
    }
    ji_material_engine_destroy(engine);
}

/* =========================================================================
 * Material transition tests
 * ========================================================================= */

static void test_material_transition(void) {
    JiMaterialEngine* engine = ji_material_engine_new();
    JiUIMaterial* mat = ji_material_flat_new(1, 0, 0, 1);
    ji_material_engine_add(engine, mat);

    ASSERT_TRUE(!ji_material_is_transitioning(mat));

    /* Start transition from flat to metal */
    ASSERT_TRUE(ji_material_engine_transition(engine, mat, JI_MATERIAL_METAL, 1.0f));
    ASSERT_TRUE(ji_material_is_transitioning(mat));

    /* Update halfway */
    ji_material_engine_update(engine, 0.5f);
    ASSERT_TRUE(ji_material_is_transitioning(mat));
    /* Metallic should be partway between 0 (flat) and 1 (metal) */
    ASSERT_TRUE(mat->metallic > 0.0f && mat->metallic < 1.0f);

    /* Complete the transition */
    ji_material_engine_update(engine, 0.6f);
    ASSERT_TRUE(!ji_material_is_transitioning(mat));
    ASSERT_TRUE(ji_ui_material_type(mat) == JI_MATERIAL_METAL);
    ASSERT_FLOAT_NEAR(mat->metallic, 1.0f, 0.01f);

    ji_ui_material_destroy(mat);
    ji_material_engine_destroy(engine);
}

/* =========================================================================
 * PBR evaluation tests
 * ========================================================================= */

static void test_pbr_evaluate_basic(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.5f, 0.5f, 0.0f, 0.5f);
    JiVec3 normal   = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};
    JiVec3 light_dir = {0, 0, 1};
    JiVec3 light_color = {1, 1, 1};

    JiVec3 result = ji_pbr_evaluate(mat, normal, view_dir, light_dir, light_color, 1.0f);
    /* With light directly facing surface, result should be positive */
    ASSERT_TRUE(result.x > 0.0f);
    ASSERT_TRUE(result.y > 0.0f);
    ASSERT_TRUE(result.z > 0.0f);

    ji_ui_material_destroy(mat);
}

static void test_pbr_evaluate_no_light(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.5f, 0.5f, 0.0f, 0.5f);
    JiVec3 normal   = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};
    /* Light from behind surface */
    JiVec3 light_dir = {0, 0, -1};
    JiVec3 light_color = {1, 1, 1};

    JiVec3 result = ji_pbr_evaluate(mat, normal, view_dir, light_dir, light_color, 1.0f);
    /* With light behind surface, diffuse should be ~0 */
    ASSERT_FLOAT_NEAR(result.x, 0.0f, 0.01f);
    ASSERT_FLOAT_NEAR(result.y, 0.0f, 0.01f);
    ASSERT_FLOAT_NEAR(result.z, 0.0f, 0.01f);

    ji_ui_material_destroy(mat);
}

static void test_pbr_evaluate_emissive(void) {
    JiUIMaterial* mat = ji_material_neon_new(1, 0, 0, 2.0f, 0.5f);
    JiVec3 normal   = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};
    JiVec3 light_dir = {0, 0, -1}; /* No direct light */
    JiVec3 light_color = {1, 1, 1};

    JiVec3 result = ji_pbr_evaluate(mat, normal, view_dir, light_dir, light_color, 1.0f);
    /* Emissive should contribute even with no direct light */
    ASSERT_TRUE(result.x > 0.0f);

    ji_ui_material_destroy(mat);
}

static void test_pbr_evaluate_ambient(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.8f, 0.8f, 0.8f, 0.0f, 0.5f);
    JiVec3 normal = {0, 1, 0};
    JiVec3 sky = {0.5f, 0.6f, 0.8f};
    JiVec3 ground = {0.2f, 0.15f, 0.1f};

    JiVec3 result = ji_pbr_evaluate_ambient(mat, normal, sky, ground);
    ASSERT_TRUE(result.x > 0.0f);
    ASSERT_TRUE(result.y > 0.0f);
    ASSERT_TRUE(result.z > 0.0f);

    ji_ui_material_destroy(mat);
}

static void test_pbr_evaluate_clearcoat(void) {
    JiUIMaterial* mat = ji_material_pbr_new(0.5f, 0.5f, 0.5f, 0.0f, 0.5f);
    ji_ui_material_set_clearcoat(mat, 1.0f, 0.05f);

    JiVec3 normal   = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};
    JiVec3 light_dir = {0, 0, 1};
    JiVec3 light_color = {1, 1, 1};

    JiVec3 cc = ji_pbr_evaluate_clearcoat(mat, normal, view_dir, light_dir, light_color, 1.0f);
    /* Clearcoat should contribute with direct light */
    ASSERT_TRUE(cc.x >= 0.0f);

    ji_ui_material_destroy(mat);
}

/* =========================================================================
 * Glass material tests
 * ========================================================================= */

static void test_glass_refract(void) {
    JiVec3 incident = {0, 0, -1};
    JiVec3 normal   = {0, 0, 1};
    float ior = 1.0f / 1.52f; /* Air to glass */

    JiVec3 refr = ji_glass_refract(incident, normal, ior);
    /* Refracted ray should still go in roughly the same direction */
    ASSERT_TRUE(refr.z < 0.0f);
}

static void test_glass_evaluate(void) {
    JiUIMaterial* mat = ji_material_glass_new(0.9f, 0.95f, 1.0f, 0.5f, 2.0f, 1.52f);
    JiVec3 normal = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};
    JiVec4 backdrop = {0.2f, 0.4f, 0.6f, 1.0f};

    JiVec3 result = ji_glass_evaluate(mat, normal, view_dir, backdrop);
    /* Should produce a valid color */
    ASSERT_TRUE(result.x >= 0.0f);
    ASSERT_TRUE(result.y >= 0.0f);
    ASSERT_TRUE(result.z >= 0.0f);

    ji_ui_material_destroy(mat);
}

static void test_glass_blur_radius(void) {
    JiUIMaterial* mat = ji_material_glass_new(0.9f, 0.95f, 1.0f, 0.5f, 5.0f, 1.52f);
    float blur = ji_glass_blur_radius(mat, 10.0f);
    ASSERT_TRUE(blur > 0.0f);

    /* Zero distance should give zero blur */
    float blur0 = ji_glass_blur_radius(mat, 0.0f);
    ASSERT_FLOAT_NEAR(blur0, 0.0f, 0.001f);

    ji_ui_material_destroy(mat);
}

static void test_glass_chromatic_aberration(void) {
    JiVec3 incident = {0, 0, -1};
    JiVec3 normal   = {0, 0, 1};
    JiVec3 r, g, b;
    ji_glass_chromatic_aberration(incident, normal, 1.0f / 1.52f, 0.005f, &r, &g, &b);
    /* All channels should be valid */
    ASSERT_TRUE(r.z < 0.0f);
    ASSERT_TRUE(g.z < 0.0f);
    ASSERT_TRUE(b.z < 0.0f);
}

/* =========================================================================
 * Neon material tests
 * ========================================================================= */

static void test_neon_glow_at(void) {
    JiUIMaterial* mat = ji_material_neon_new(0, 1, 0, 2.0f, 0.5f);
    float glow_edge = ji_neon_glow_at(mat, 0.0f);
    float glow_far  = ji_neon_glow_at(mat, 20.0f);
    /* Glow at edge should be stronger than far away */
    ASSERT_TRUE(glow_edge >= glow_far);
    ji_ui_material_destroy(mat);
}

static void test_neon_evaluate(void) {
    JiUIMaterial* mat = ji_material_neon_new(1, 0, 0, 2.0f, 0.5f);
    JiVec3 color = ji_neon_evaluate(mat, 0.0f);
    /* At edge, should produce red glow */
    ASSERT_TRUE(color.x >= 0.0f);
    ji_ui_material_destroy(mat);
}

static void test_neon_bloom_contribution(void) {
    JiUIMaterial* mat = ji_material_neon_new(1, 1, 1, 2.0f, 0.3f);
    JiVec3 bright = {1.0f, 1.0f, 1.0f};
    JiVec3 dim    = {0.1f, 0.1f, 0.1f};

    JiVec3 bloom_bright = ji_neon_bloom_contribution(mat, bright);
    JiVec3 bloom_dim    = ji_neon_bloom_contribution(mat, dim);

    /* Bright color should bloom more than dim */
    ASSERT_TRUE(bloom_bright.x >= bloom_dim.x);
    ji_ui_material_destroy(mat);
}

static void test_neon_edge_intensity(void) {
    float on_edge = ji_neon_edge_intensity(0.0f, 5.0f);
    float off_edge = ji_neon_edge_intensity(5.0f, 5.0f);
    ASSERT_FLOAT_NEAR(on_edge, 1.0f, 0.001f);
    ASSERT_FLOAT_NEAR(off_edge, 0.0f, 0.001f);
}

/* =========================================================================
 * Lighting system tests
 * ========================================================================= */

static void test_lighting_lifecycle(void) {
    JiLightingSystem* sys = ji_lighting_new();
    ASSERT_TRUE(sys != NULL);
    ASSERT_TRUE(ji_lighting_count(sys) == 0);
    ji_lighting_destroy(sys);
}

static void test_lighting_add_remove(void) {
    JiLightingSystem* sys = ji_lighting_new();
    Ji3DLight* light1 = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);
    Ji3DLight* light2 = ji_3d_light_new(JI_3D_LIGHT_POINT);

    ASSERT_TRUE(ji_lighting_add(sys, light1));
    ASSERT_TRUE(ji_lighting_add(sys, light2));
    ASSERT_TRUE(ji_lighting_count(sys) == 2);

    ASSERT_TRUE(ji_lighting_remove(sys, light1));
    ASSERT_TRUE(ji_lighting_count(sys) == 1);

    ji_3d_light_destroy(light1);
    ji_3d_light_destroy(light2);
    ji_lighting_destroy(sys);
}

static void test_lighting_set_ambient(void) {
    JiLightingSystem* sys = ji_lighting_new();
    ji_lighting_set_ambient(sys, 0.3f, 0.4f, 0.5f, 0.6f);
    /* No getter, but should not crash */
    ji_lighting_destroy(sys);
}

static void test_lighting_compute_directional(void) {
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);
    ji_3d_light_set_direction(light, 0, -1, 0);
    ji_3d_light_set_color(light, 1, 1, 1, 1.0f);

    JiVec3 dir;
    float intensity;
    ji_lighting_compute(light, (JiVec3){0, 0, 0}, &dir, &intensity);
    ASSERT_TRUE(intensity > 0.0f);

    ji_3d_light_destroy(light);
}

static void test_lighting_compute_point(void) {
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_POINT);
    ji_3d_light_set_position(light, 0, 5, 0);
    ji_3d_light_set_color(light, 1, 1, 1, 1.0f);
    light->range = 10.0f;

    JiVec3 dir;
    float intensity;
    ji_lighting_compute(light, (JiVec3){0, 0, 0}, &dir, &intensity);
    /* Point light at distance 5 should have some intensity */
    ASSERT_TRUE(intensity > 0.0f);
    ASSERT_TRUE(intensity < 1.0f); /* attenuated */

    ji_3d_light_destroy(light);
}

static void test_lighting_evaluate(void) {
    JiLightingSystem* sys = ji_lighting_new();
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);
    ji_3d_light_set_direction(light, 0, 0, -1);
    ji_3d_light_set_color(light, 1, 1, 1, 1.0f);
    ji_lighting_add(sys, light);

    JiUIMaterial* mat = ji_material_pbr_new(0.8f, 0.8f, 0.8f, 0.0f, 0.5f);
    JiVec3 pos = {0, 0, 0};
    JiVec3 normal = {0, 0, 1};
    JiVec3 view_dir = {0, 0, 1};

    JiVec3 result = ji_lighting_evaluate(sys, mat, pos, normal, view_dir);
    ASSERT_TRUE(result.x > 0.0f);
    ASSERT_TRUE(result.y > 0.0f);
    ASSERT_TRUE(result.z > 0.0f);

    ji_ui_material_destroy(mat);
    ji_3d_light_destroy(light);
    ji_lighting_destroy(sys);
}

static void test_lighting_evaluate_2d(void) {
    JiLightingSystem* sys = ji_lighting_new();
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);
    ji_3d_light_set_direction(light, 0, 0, -1);
    ji_3d_light_set_color(light, 1, 1, 1, 1.0f);
    ji_lighting_add(sys, light);

    JiUIMaterial* mat = ji_material_pbr_new(0.8f, 0.8f, 0.8f, 0.0f, 0.5f);
    JiVec3 light_dir = {0, 0, 1};

    JiVec3 result = ji_lighting_evaluate_2d(sys, mat, light_dir);
    ASSERT_TRUE(result.x > 0.0f);

    ji_ui_material_destroy(mat);
    ji_3d_light_destroy(light);
    ji_lighting_destroy(sys);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== JiUI Material Engine Tests ===\n\n");

    printf("--- Material Creation ---\n");
    TEST(test_material_flat_new);
    TEST(test_material_glass_new);
    TEST(test_material_metal_new);
    TEST(test_material_neon_new);
    TEST(test_material_holographic_new);
    TEST(test_material_pbr_new);
    TEST(test_material_carbon_new);

    printf("\n--- Material Properties ---\n");
    TEST(test_material_set_color);
    TEST(test_material_set_metallic_roughness);
    TEST(test_material_set_emissive);
    TEST(test_material_set_clearcoat);
    TEST(test_material_set_glass);
    TEST(test_material_set_wireframe);
    TEST(test_material_set_double_sided);
    TEST(test_material_set_animation);

    printf("\n--- Material Engine ---\n");
    TEST(test_material_engine_lifecycle);
    TEST(test_material_engine_add_remove);
    TEST(test_material_engine_update);
    TEST(test_material_engine_grow);

    printf("\n--- Material Transitions ---\n");
    TEST(test_material_transition);

    printf("\n--- PBR Evaluation ---\n");
    TEST(test_pbr_evaluate_basic);
    TEST(test_pbr_evaluate_no_light);
    TEST(test_pbr_evaluate_emissive);
    TEST(test_pbr_evaluate_ambient);
    TEST(test_pbr_evaluate_clearcoat);

    printf("\n--- Glass Material ---\n");
    TEST(test_glass_refract);
    TEST(test_glass_evaluate);
    TEST(test_glass_blur_radius);
    TEST(test_glass_chromatic_aberration);

    printf("\n--- Neon Material ---\n");
    TEST(test_neon_glow_at);
    TEST(test_neon_evaluate);
    TEST(test_neon_bloom_contribution);
    TEST(test_neon_edge_intensity);

    printf("\n--- Lighting System ---\n");
    TEST(test_lighting_lifecycle);
    TEST(test_lighting_add_remove);
    TEST(test_lighting_set_ambient);
    TEST(test_lighting_compute_directional);
    TEST(test_lighting_compute_point);
    TEST(test_lighting_evaluate);
    TEST(test_lighting_evaluate_2d);

    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
