/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_material_neon.c
 * @brief Neon material — emissive edges, bloom threshold, glow falloff,
 *        and animated pulse effects.
 */

#include "jiui/ji_material.h"
#include "jiui/ji_3d.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * Neon Internal Helpers
 * ========================================================================= */

static float neon_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Gaussian glow falloff */
static float neon_gaussian(float x, float sigma) {
    return expf(-(x * x) / (2.0f * sigma * sigma));
}

/* =========================================================================
 * Neon Public API
 * ========================================================================= */

/**
 * Evaluate neon material glow at a given distance from the edge.
 *
 * @param mat          Neon material.
 * @param edge_distance Distance from the neon edge (0 = on edge, >0 = away).
 * @return             Glow intensity (0..1).
 */
JI_API float ji_neon_glow_at(const JiUIMaterial* mat, float edge_distance) {
    if (!mat) return 0.0f;

    /* Core glow: bright at edge, falls off with Gaussian */
    float sigma = 3.0f + mat->roughness * 10.0f;
    float core = neon_gaussian(edge_distance, sigma);

    /* Apply emissive intensity */
    float intensity = core * mat->emissive;

    /* Apply bloom threshold — only emit above threshold */
    if (mat->bloom_threshold > 0.0f) {
        intensity = intensity > mat->bloom_threshold ? intensity : 0.0f;
    }

    return neon_clampf(intensity, 0.0f, 1.0f);
}

/**
 * Evaluate neon color at a point, including pulse animation.
 *
 * @param mat          Neon material.
 * @param edge_distance Distance from the neon edge.
 * @return             Neon color (RGB in JiVec3).
 */
JI_API JiVec3 ji_neon_evaluate(const JiUIMaterial* mat, float edge_distance) {
    if (!mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    float glow = ji_neon_glow_at(mat, edge_distance);

    /* Pulse animation — sinusoidal modulation */
    float pulse = 1.0f;
    if (mat->animation_speed > 0.0f) {
        pulse = 0.7f + 0.3f * sinf(mat->animation_time * mat->animation_speed * 2.0f * (float)M_PI);
    }

    JiVec3 color = {
        mat->emissive_color.x * glow * pulse,
        mat->emissive_color.y * glow * pulse,
        mat->emissive_color.z * glow * pulse
    };

    return color;
}

/**
 * Compute bloom contribution for a neon material.
 * Returns the color that should be added to the bloom buffer.
 *
 * @param mat          Neon material.
 * @param base_color   Base rendered color at this pixel.
 * @return             Bloom contribution (RGB in JiVec3).
 */
JI_API JiVec3 ji_neon_bloom_contribution(const JiUIMaterial* mat, JiVec3 base_color) {
    if (!mat) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    /* Luminance of the base color */
    float lum = 0.2126f * base_color.x + 0.7152f * base_color.y + 0.0722f * base_color.z;

    /* Only bloom above threshold */
    if (lum < mat->bloom_threshold) {
        JiVec3 black = {0, 0, 0};
        return black;
    }

    /* Bloom intensity scales with how far above threshold */
    float bloom_strength = (lum - mat->bloom_threshold) / (1.0f - mat->bloom_threshold + 0.001f);
    bloom_strength = neon_clampf(bloom_strength, 0.0f, 1.0f);

    JiVec3 bloom = {
        base_color.x * bloom_strength * mat->emissive,
        base_color.y * bloom_strength * mat->emissive,
        base_color.z * bloom_strength * mat->emissive
    };
    return bloom;
}

/**
 * Compute neon edge detection for a UI element.
 * Given the distance field value, returns the edge intensity.
 *
 * @param distance_field  Signed distance field value (negative = inside, positive = outside).
 * @param edge_width       Width of the neon edge in pixels.
 * @return                 Edge intensity (0..1).
 */
JI_API float ji_neon_edge_intensity(float distance_field, float edge_width) {
    if (edge_width <= 0.0f) return 0.0f;

    /* Edge is where distance field crosses zero */
    float normalized = distance_field / edge_width;
    float edge = 1.0f - neon_clampf(fabsf(normalized), 0.0f, 1.0f);
    return edge * edge; /* squared for sharper falloff */
}
