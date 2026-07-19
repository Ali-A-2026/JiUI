/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_neural.c
 * @brief Neural UI rendering implementation — EMA-based render time tracking,
 *        Markov chain interaction prediction, adaptive animation timing,
 *        and WCAG contrast accessibility evaluation.
 */

#include "jiui/ji_neural.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================================
 * Helpers
 * ========================================================================= */

static float neural_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float neural_luminance(float r, float g, float b) {
    /* sRGB to relative luminance (WCAG formula) */
    float lr = (r <= 0.03928f) ? r / 12.92f : powf((r + 0.055f) / 1.055f, 2.4f);
    float lg = (g <= 0.03928f) ? g / 12.92f : powf((g + 0.055f) / 1.055f, 2.4f);
    float lb = (b <= 0.03928f) ? b / 12.92f : powf((b + 0.055f) / 1.055f, 2.4f);
    return 0.2126f * lr + 0.7152f * lg + 0.0722f * lb;
}

/* =========================================================================
 * Neural Model — Lifecycle
 * ========================================================================= */

JI_API JiNeuralModel* ji_neural_new(void) {
    JiNeuralModel* model = JI_NEW(JiNeuralModel);
    if (!model) return NULL;
    memset(model, 0, sizeof(JiNeuralModel));

    model->render_time_capacity = 120; /* ~2 seconds at 60fps */
    model->render_times = JI_NEW_ARRAY(float, model->render_time_capacity);
    model->render_time_count = 0;
    model->render_time_index = 0;
    model->avg_render_time = 16.67f; /* ~60fps default */
    model->render_time_std = 0.0f;

    model->interaction_capacity = 256;
    model->interaction_history = JI_NEW_ARRAY(int, model->interaction_capacity);
    model->interaction_count = 0;
    model->interaction_index = 0;
    memset(model->transition_matrix, 0, sizeof(model->transition_matrix));

    model->preferred_duration = 300.0f; /* 300ms default */
    model->duration_adjustment = 1.0f;

    model->auto_font_scale = 1.0f;
    model->auto_contrast = 0.0f;
    model->user_reading_speed = 200.0f; /* ~200 chars/sec average */

    model->learning_rate = 0.1f;
    model->enabled = true;

    if (!model->render_times || !model->interaction_history) {
        ji_neural_destroy(model);
        return NULL;
    }
    return model;
}

JI_API void ji_neural_destroy(JiNeuralModel* model) {
    if (!model) return;
    if (model->render_times) ji_free(model->render_times);
    if (model->interaction_history) ji_free(model->interaction_history);
    ji_free(model);
}

JI_API void ji_neural_set_enabled(JiNeuralModel* model, bool enabled) {
    if (!model) return;
    model->enabled = enabled;
}

JI_API void ji_neural_set_learning_rate(JiNeuralModel* model, float rate) {
    if (!model) return;
    model->learning_rate = neural_clampf(rate, 0.0f, 1.0f);
}

/* =========================================================================
 * Layout Optimization
 * ========================================================================= */

JI_API void ji_neural_record_render_time(JiNeuralModel* model, float time_ms) {
    if (!model || !model->enabled || time_ms <= 0.0f) return;

    /* Store in ring buffer */
    model->render_times[model->render_time_index] = time_ms;
    model->render_time_index = (model->render_time_index + 1) % model->render_time_capacity;
    if (model->render_time_count < model->render_time_capacity) {
        model->render_time_count++;
    }

    /* Update EMA */
    float alpha = model->learning_rate;
    model->avg_render_time = (1.0f - alpha) * model->avg_render_time + alpha * time_ms;

    /* Update standard deviation estimate (running) */
    float diff = time_ms - model->avg_render_time;
    model->render_time_std = (1.0f - alpha) * model->render_time_std + alpha * fabsf(diff);
}

JI_API float ji_neural_avg_render_time(const JiNeuralModel* model) {
    return model ? model->avg_render_time : 0.0f;
}

JI_API float ji_neural_render_time_std(const JiNeuralModel* model) {
    return model ? model->render_time_std : 0.0f;
}

JI_API float ji_neural_suggest_layout_complexity(const JiNeuralModel* model,
                                                    float target_fps) {
    if (!model || target_fps <= 0.0f) return 1.0f;

    float target_frame_time = 1000.0f / target_fps; /* ms per frame */
    float avg = model->avg_render_time;

    if (avg <= target_frame_time) return 1.0f; /* No reduction needed */

    /* Suggest complexity reduction proportional to how much we exceed target */
    float excess = (avg - target_frame_time) / target_frame_time;
    return neural_clampf(1.0f - excess * 0.5f, 0.0f, 1.0f);
}

/* =========================================================================
 * Interaction Prediction
 * ========================================================================= */

JI_API void ji_neural_record_interaction(JiNeuralModel* model, int interaction_id) {
    if (!model || !model->enabled) return;
    if (interaction_id < 0 || interaction_id >= 16) return;

    /* Record in history */
    if (model->interaction_count > 0) {
        int prev = model->interaction_history[
            (model->interaction_index - 1 + model->interaction_capacity) % model->interaction_capacity
        ];
        if (prev >= 0 && prev < 16) {
            model->transition_matrix[prev][interaction_id]++;
        }
    }

    model->interaction_history[model->interaction_index] = interaction_id;
    model->interaction_index = (model->interaction_index + 1) % model->interaction_capacity;
    if (model->interaction_count < model->interaction_capacity) {
        model->interaction_count++;
    }
}

JI_API int ji_neural_predict_next_interaction(const JiNeuralModel* model,
                                                  int current_interaction_id) {
    if (!model || current_interaction_id < 0 || current_interaction_id >= 16) return -1;

    int best = -1;
    int best_count = 0;
    for (int i = 0; i < 16; i++) {
        if (model->transition_matrix[current_interaction_id][i] > best_count) {
            best_count = model->transition_matrix[current_interaction_id][i];
            best = i;
        }
    }
    return best;
}

JI_API float ji_neural_interaction_probability(const JiNeuralModel* model,
                                                    int from_id, int to_id) {
    if (!model || from_id < 0 || from_id >= 16 || to_id < 0 || to_id >= 16) return 0.0f;

    int total = 0;
    for (int i = 0; i < 16; i++) {
        total += model->transition_matrix[from_id][i];
    }
    if (total == 0) return 0.0f;
    return (float)model->transition_matrix[from_id][to_id] / (float)total;
}

/* =========================================================================
 * Smart Animation Timing
 * ========================================================================= */

JI_API void ji_neural_record_animation(JiNeuralModel* model, float duration) {
    if (!model || !model->enabled || duration <= 0.0f) return;

    /* EMA of preferred duration */
    float alpha = model->learning_rate;
    model->preferred_duration = (1.0f - alpha) * model->preferred_duration + alpha * duration;

    /* Adjust based on user behavior — if user frequently interrupts animations,
       they may prefer shorter durations. This is a simplified heuristic. */
    model->duration_adjustment = neural_clampf(model->duration_adjustment, 0.5f, 2.0f);
}

JI_API float ji_neural_suggested_duration(const JiNeuralModel* model) {
    if (!model) return 300.0f;
    return model->preferred_duration * model->duration_adjustment;
}

/* =========================================================================
 * Accessibility
 * ========================================================================= */

JI_API void ji_neural_record_reading_speed(JiNeuralModel* model, float chars_per_sec) {
    if (!model || !model->enabled || chars_per_sec <= 0.0f) return;

    float alpha = model->learning_rate;
    model->user_reading_speed = (1.0f - alpha) * model->user_reading_speed + alpha * chars_per_sec;

    /* Auto-adjust font scale based on reading speed */
    /* Slower readers get larger fonts */
    if (model->user_reading_speed < 100.0f) {
        model->auto_font_scale = 1.25f;
    } else if (model->user_reading_speed < 150.0f) {
        model->auto_font_scale = 1.1f;
    } else if (model->user_reading_speed > 300.0f) {
        model->auto_font_scale = 0.95f;
    } else {
        model->auto_font_scale = 1.0f;
    }
}

JI_API float ji_neural_auto_font_scale(const JiNeuralModel* model) {
    return model ? model->auto_font_scale : 1.0f;
}

JI_API float ji_neural_auto_contrast(const JiNeuralModel* model) {
    return model ? model->auto_contrast : 0.0f;
}

JI_API float ji_neural_contrast_ratio(float fg_r, float fg_g, float fg_b,
                                         float bg_r, float bg_g, float bg_b) {
    float l1 = neural_luminance(fg_r, fg_g, fg_b);
    float l2 = neural_luminance(bg_r, bg_g, bg_b);

    float lighter = (l1 > l2) ? l1 : l2;
    float darker = (l1 > l2) ? l2 : l1;

    return (lighter + 0.05f) / (darker + 0.05f);
}

JI_API void ji_neural_suggest_fg_color(float bg_r, float bg_g, float bg_b,
                                         float* out_r, float* out_g, float* out_b) {
    if (!out_r || !out_g || !out_b) return;

    float bg_lum = neural_luminance(bg_r, bg_g, bg_b);

    /* If background is dark, suggest white; if light, suggest black */
    if (bg_lum < 0.18f) {
        *out_r = 1.0f; *out_g = 1.0f; *out_b = 1.0f;
    } else {
        *out_r = 0.0f; *out_g = 0.0f; *out_b = 0.0f;
    }
}
